#define _CRT_SECURE_NO_WARNINGS
#define CGLTF_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include "cgltf.h"
#include "model_loader.h"
#include "array.h"
#include "brh_mesh.h"

/*
* Loads an OBJ file into a mesh struct and returns true if successful
* 
* @param file_path The path to the OBJ file
* @param mesh The mesh struct to load the OBJ file into
* 
* @return true if the OBJ file was loaded successfully, false otherwise
*/
bool load_obj(const char* file_path, brh_mesh* mesh, bool isRightHanded)
{
    FILE* file;
    file = fopen(file_path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        return false;
    }

    char line[1024];
    while (fgets(line, 1024, file) != NULL)
    {
        if (strncmp(line, "v ", 2) == 0)
        {
            brh_vector3 vertex;
            errno_t err = sscanf(line, "v %f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            if (err != 3)
            {
                fprintf(stderr, "Error parsing vertex data\n");
                return false;
            }
            if (isRightHanded)
            {
                vertex.z = -vertex.z;
            }
            array_push(mesh->vertices, vertex);
        }
        else if (line[0] == 'f')
        {
            brh_face face;

            // First try parsing with slashes (f 1/1/1 2/2/1 3/3/1 format)
            errno_t err = sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d\n", &face.a, &face.b, &face.c);

            // If that didn't work, try parsing with just spaces (f 1 2 3 format)
            if (err != 3)
            {
                err = sscanf(line, "f %d %d %d\n", &face.a, &face.b, &face.c);
                if (err != 3)
                {
                    fprintf(stderr, "Error parsing face data\n");
                    return false;
                }
            }

            if (isRightHanded)
            {
                int temp = face.a;
                face.a = face.c;
                face.c = temp;
				face.color = 0xFFFFFFFF;
            }
            array_push(mesh->faces, face);
        }
    }

    fclose(file);
    return true;
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
