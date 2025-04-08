#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define CGLTF_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include "cgltf.h"
#include "model_loader.h"
#include "array.h"
#include "brh_mesh.h"

/**
 * @brief Clean up allocated mesh resources
 *
 * @param mesh Mesh to clean up
 */
static void cleanup_mesh(brh_mesh* mesh)
{
    if (mesh->vertices) array_free(mesh->vertices);
    if (mesh->texcoords) array_free(mesh->texcoords);
    if (mesh->faces) array_free(mesh->faces);
    if (mesh->normals) array_free(mesh->normals);

    mesh->vertices = NULL;
    mesh->texcoords = NULL;
    mesh->faces = NULL;
    mesh->normals = NULL;
}

/**
 * @brief Parse face data from a line in an OBJ file.
 *
 * Handles various face formats: v/vt/vn, v/vt, v//vn, v
 *
 * @param line Line from OBJ file starting with 'f '
 * @param mesh Mesh to add the face to
 * @param isRightHanded Whether to convert winding order
 * @return true if parsing succeeded, false otherwise
 */
static bool parse_face(const char* line, brh_mesh* mesh, bool isRightHanded)
{
    brh_face face = { 0 };
    face.color = 0xFFFFFFFF;  // Default color (white)

    int v[3] = { 0 };      // Vertex indices
    int vt[3] = { 0 };     // Texture indices
    int vn[3] = { 0 };     // Normal indices
    bool success = false;

    // Try each format in order of most to least complex

    // Format 1: f v/vt/vn v/vt/vn v/vt/vn (vertices/textures/normals)
    if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
        &v[0], &vt[0], &vn[0],
        &v[1], &vt[1], &vn[1],
        &v[2], &vt[2], &vn[2]) == 9) {

        face.a = v[0] - 1; face.a_vt = vt[0] - 1; face.a_vn = vn[0] - 1;
        face.b = v[1] - 1; face.b_vt = vt[1] - 1; face.b_vn = vn[1] - 1;
        face.c = v[2] - 1; face.c_vt = vt[2] - 1; face.c_vn = vn[2] - 1;
        success = true;
    }
    // Format 2: f v//vn v//vn v//vn (vertices/normals, no textures)
    else if (sscanf(line, "f %d//%d %d//%d %d//%d",
        &v[0], &vn[0],
        &v[1], &vn[1],
        &v[2], &vn[2]) == 6) {

        face.a = v[0] - 1; face.a_vt = 0; face.a_vn = vn[0] - 1;
        face.b = v[1] - 1; face.b_vt = 0; face.b_vn = vn[1] - 1;
        face.c = v[2] - 1; face.c_vt = 0; face.c_vn = vn[2] - 1;
        success = true;
    }
    // Format 3: f v/vt v/vt v/vt (vertices/textures, no normals)
    else if (sscanf(line, "f %d/%d %d/%d %d/%d",
        &v[0], &vt[0],
        &v[1], &vt[1],
        &v[2], &vt[2]) == 6) {

        face.a = v[0] - 1; face.a_vt = vt[0] - 1; face.a_vn = 0;
        face.b = v[1] - 1; face.b_vt = vt[1] - 1; face.b_vn = 0;
        face.c = v[2] - 1; face.c_vt = vt[2] - 1; face.c_vn = 0;
        success = true;
    }
    // Format 4: f v v v (vertices only)
    else if (sscanf(line, "f %d %d %d", &v[0], &v[1], &v[2]) == 3) {

        face.a = v[0] - 1; face.a_vt = 0; face.a_vn = 0;
        face.b = v[1] - 1; face.b_vt = 0; face.b_vn = 0;
        face.c = v[2] - 1; face.c_vt = 0; face.c_vn = 0;
        success = true;
    }

    if (!success) {
        return false;
    }

    // Convert winding order if needed
    if (isRightHanded) {
        // Swap vertices a and c and their associated attributes
        int temp_v = face.a;
        face.a = face.c;
        face.c = temp_v;

        int temp_vt = face.a_vt;
        face.a_vt = face.c_vt;
        face.c_vt = temp_vt;

        int temp_vn = face.a_vn;
        face.a_vn = face.c_vn;
        face.c_vn = temp_vn;
    }

    array_push(mesh->faces, face);
    return true;
}

/**
 * @brief Loads vertex positions, texture coordinates, and face indices from an OBJ file.
 *
 * Handles common face formats: v/vt/vn, v/vt, v//vn, v.
 * Populates the mesh struct's vertices, texcoords, and faces arrays.
 * Converts coordinates to left-handed if isRightHanded is true.
 *
 * @param file_path Path to the OBJ file.
 * @param mesh Pointer to the mesh structure to populate.
 * @param isRightHanded If true, converts Z-coordinates and reverses face winding order.
 * @return true if loading was successful, false otherwise.
 */
bool load_obj(const char* file_path, brh_mesh* mesh, bool isRightHanded)
{
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        return false;
    }

    // Initialize dynamic arrays in the mesh struct
    mesh->vertices = NULL;
    mesh->texcoords = NULL;
    mesh->faces = NULL;
    mesh->normals = NULL;

    char line[1024];
    bool success = true;

    while (fgets(line, 1024, file) != NULL && success) {
        // Parse vertex positions
        if (strncmp(line, "v ", 2) == 0) {
            brh_vector3 vertex;
            if (sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) != 3) {
                fprintf(stderr, "Error parsing vertex data: %s", line);
                success = false;
                break;
            }

            if (isRightHanded) {
                vertex.z = -vertex.z;  // Convert coordinate system
            }
            array_push(mesh->vertices, vertex);
        }
        // Parse texture coordinates
        else if (strncmp(line, "vt ", 3) == 0) {
            brh_texel texcoord;
            if (sscanf(line, "vt %f %f", &texcoord.u, &texcoord.v) != 2) {
                fprintf(stderr, "Error parsing texture coordinate data: %s", line);
                success = false;
                break;
            }
            array_push(mesh->texcoords, texcoord);
        }
        // Parse normals
        else if (strncmp(line, "vn ", 3) == 0) {
            brh_vector3 normal;
            if (sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z) != 3) {
                fprintf(stderr, "Error parsing normal data: %s", line);
                success = false;
                break;
            }

            if (isRightHanded) {
                normal.z = -normal.z;  // Convert coordinate system
            }
            array_push(mesh->normals, normal);
        }
        // Parse face data
        else if (line[0] == 'f' && line[1] == ' ') {
            if (!parse_face(line, mesh, isRightHanded)) {
                fprintf(stderr, "Error parsing face data: %s", line);
                success = false;
                break;
            }
        }
    }

    // If there was an error, clean up allocated memory
    if (!success) {
        cleanup_mesh(mesh);
    }

    fclose(file);
    return success;
}

/*
* Loads a glTF file into a mesh struct and returns true if successful
* 
* @param file_path The path to the glTF file
* @param mesh The mesh struct to load the glTF file into
* 
* @return true if the glTF file was loaded successfully, false otherwise
*/
bool load_gltf(const char* file_path, brh_mesh* mesh)
{
    cgltf_options options = { 0 };
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, file_path, &data);
    if (result != cgltf_result_success)
    {
        fprintf(stderr, "Error loading glTF file: %s\n", file_path);
        return false;
    }

    result = cgltf_load_buffers(&options, data, file_path);
    if (result != cgltf_result_success)
    {
        fprintf(stderr, "Error loading glTF buffers: %s\n", file_path);
        cgltf_free(data);
        return false;
    }

    // Extract vertex and normal data
    for (cgltf_size i = 0; i < data->meshes_count; ++i)
    {
        cgltf_mesh* gltf_mesh = &data->meshes[i];
        for (cgltf_size j = 0; j < gltf_mesh->primitives_count; ++j)
        {
            cgltf_primitive* primitive = &gltf_mesh->primitives[j];
            cgltf_accessor* position_accessor = NULL;
            cgltf_accessor* normal_accessor = NULL;
            cgltf_accessor* index_accessor = primitive->indices;

            for (cgltf_size k = 0; k < primitive->attributes_count; ++k)
            {
                if (primitive->attributes[k].type == cgltf_attribute_type_position)
                {
                    position_accessor = primitive->attributes[k].data;
                }
                else if (primitive->attributes[k].type == cgltf_attribute_type_normal)
                {
                    normal_accessor = primitive->attributes[k].data;
                }
            }

            if (position_accessor)
            {
                for (cgltf_size v = 0; v < position_accessor->count; ++v)
                {
                    cgltf_float position[3];
                    cgltf_accessor_read_float(position_accessor, v, position, 3);
                    // Convert from right-handed to left-handed coordinate system
                    brh_vector3 vertex = { position[0], position[1], -position[2] };
                    array_push(mesh->vertices, vertex);
                }
            }

            //if (normal_accessor)
            //{
            //    for (cgltf_size n = 0; n < normal_accessor->count; ++n)
            //    {
            //        cgltf_float normal[3];
            //        cgltf_accessor_read_float(normal_accessor, n, normal, 3);
            //        // Convert from right-handed to left-handed coordinate system
            //        brh_vector3 normal_vector = { normal[0], normal[1], -normal[2] };
            //        array_push(mesh->normals, normal_vector);
            //    }
            //}

            if (index_accessor)
            {
                for (cgltf_size f = 0; f < index_accessor->count; f += 3)
                {
                    brh_face face;
                    // Adjust winding order from counter-clockwise to clockwise
                    face.a = (int)cgltf_accessor_read_index(index_accessor, f) + 1;
                    face.c = (int)cgltf_accessor_read_index(index_accessor, f + 1) + 1;
                    face.b = (int)cgltf_accessor_read_index(index_accessor, f + 2) + 1;
                    array_push(mesh->faces, face);
                }
            }
        }
    }

    cgltf_free(data);
    return true;
}
