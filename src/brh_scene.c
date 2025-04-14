#include <stdio.h>
#include "brh_scene.h"
#include "brh_matrix.h"
#include "brh_clipping.h"
#include "brh_display.h"
#include "brh_light.h"
#include "array.h"
#include "math_utils.h"

#define MAX_SCENE_OBJECTS 256  // Maximum number of objects in the scene

static brh_renderable_handle scene_objects[MAX_SCENE_OBJECTS];
static int scene_object_count = 0;
static brh_mouse_camera* active_camera = NULL;
static brh_mat4 perspective_projection_matrix;
static brh_mat4 camera_matrix;

bool initialize_scene_system(void)
{
    scene_object_count = 0;
    active_camera = NULL;

    // Initialize projection matrix
    float fov_radians = degrees_to_radians(get_frustum_fov_y());
    float aspect_ratio = (float)get_window_width() / (float)get_window_height();
    perspective_projection_matrix = mat4_create_perspective_projection(
        fov_radians,
        aspect_ratio,
        get_frustum_near_plane(),
        get_frustum_far_plane()
    );

    return true;
}

void cleanup_scene_system(void)
{
    // Just reset the scene object count
    scene_object_count = 0;
    active_camera = NULL;
}

bool add_to_scene(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle) {
        return false;
    }

    if (scene_object_count >= MAX_SCENE_OBJECTS) {
        fprintf(stderr, "Error: Maximum number of scene objects (%d) reached\n", MAX_SCENE_OBJECTS);
        return false;
    }

    scene_objects[scene_object_count++] = renderable_handle;
    return true;
}

void remove_from_scene(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle) {
        return;
    }

    // Find the object in the scene
    for (int i = 0; i < scene_object_count; i++) {
        if (scene_objects[i] == renderable_handle) {
            // Remove it by shifting all subsequent objects down
            for (int j = i; j < scene_object_count - 1; j++) {
                scene_objects[j] = scene_objects[j + 1];
            }
            scene_object_count--;
            break;
        }
    }
}

void set_scene_camera(brh_mouse_camera* camera)
{
    active_camera = camera;
}

void update_scene(float delta_time)
{
    // Update all renderable objects
    //update_renderables(delta_time);

    // Update camera matrix
    if (active_camera) {
        camera_matrix = get_mouse_camera_view_matrix(active_camera);
    }
    else {
        camera_matrix = mat4_identity();
    }
}

int render_scene(brh_triangle* triangle_buffer, int buffer_capacity)
{
    if (!triangle_buffer || buffer_capacity <= 0) {
        return 0;
    }

    int triangle_count = 0;

    // Process each object in the scene
    for (int i = 0; i < scene_object_count && triangle_count < buffer_capacity; i++) {
        brh_renderable_handle renderable = scene_objects[i];
        brh_mesh* mesh = get_mesh_data(get_renderable_mesh(renderable));
        brh_texture_handle texture = get_renderable_texture(renderable);
        brh_mat4 world_matrix = get_renderable_world_matrix(renderable);

        if (!mesh) {
            continue;
        }

        int num_faces = array_length(mesh->faces);
        int num_texcoords = array_length(mesh->texcoords);

        // Process each face
        for (int j = 0; j < num_faces && triangle_count < buffer_capacity; j++) {
            brh_face face = mesh->faces[j];
            brh_vector3 face_vertices_model[3];
            brh_vector4 face_vertices_world[3];
            brh_vector4 face_vertices_camera[3];
            brh_vertex triangle_vertices[3];
            brh_texel face_texels[3] = { {0, 0}, {0, 0}, {0, 0} };

            // Get vertices from mesh
            face_vertices_model[0] = mesh->vertices[face.a];
            face_vertices_model[1] = mesh->vertices[face.b];
            face_vertices_model[2] = mesh->vertices[face.c];

            // Get texture coordinates if available
            if (num_texcoords > 0) {
                if (face.a_vt < num_texcoords) face_texels[0] = mesh->texcoords[face.a_vt];
                if (face.b_vt < num_texcoords) face_texels[1] = mesh->texcoords[face.b_vt];
                if (face.c_vt < num_texcoords) face_texels[2] = mesh->texcoords[face.c_vt];
            }

            // Transform vertices through the rendering pipeline
            for (int k = 0; k < 3; k++) {
                // Transform from model space to world space
                brh_vector4 world_vertex = vec4_from_vec3(face_vertices_model[k]);
                mat4_mul_vec4_ref(&world_matrix, &world_vertex);
                face_vertices_world[k] = world_vertex;

                // Transform from world space to camera (view) space
                brh_vector4 camera_vertex = face_vertices_world[k];
                mat4_mul_vec4_ref(&camera_matrix, &camera_vertex);
                face_vertices_camera[k] = camera_vertex;

                // Transform from camera space to clip space
                brh_vector4 clip_vertex = mat4_mul_vec4(
                    &perspective_projection_matrix,
                    camera_vertex
                );

                // Store inverse W for perspective correction
                float original_w = clip_vertex.w;
                triangle_vertices[k].inv_w = 1.0f / original_w;

                // Save the clip space position in the vertex (for clipping)
                triangle_vertices[k].position = clip_vertex;

                // Assign texture coordinates
                triangle_vertices[k].texel = face_texels[k];
            }

            // Perform backface culling
            if (get_cull_method() == CULL_BACKFACE) {
                brh_vector3 vecA_camera = vec3_from_vec4(face_vertices_camera[0]);
                brh_vector3 vecB_camera = vec3_from_vec4(face_vertices_camera[1]);
                brh_vector3 vecC_camera = vec3_from_vec4(face_vertices_camera[2]);
                brh_vector3 face_normal = get_face_normal(vecA_camera, vecB_camera, vecC_camera);

                // Calculate view vector (from face to camera)
                brh_vector3 origin = { 0.0f, 0.0f, 0.0f };
                brh_vector3 view_vector = vec3_subtract(origin, vecA_camera);
                float angle_dot_product = vec3_dot(face_normal, view_vector);

                if (angle_dot_product < 0) {
                    continue; // Skip this face if it's backfacing
                }
            }

            // Calculate face normal for lighting
            brh_vector3 vecA_world = vec3_from_vec4(face_vertices_world[0]);
            brh_vector3 vecB_world = vec3_from_vec4(face_vertices_world[1]);
            brh_vector3 vecC_world = vec3_from_vec4(face_vertices_world[2]);
            brh_vector3 face_normal = get_face_normal(vecA_world, vecB_world, vecC_world);

            // Apply flat shading
            uint32_t triangle_color = calculate_flat_shading_color(face_normal, face.color);

            // Set normal for all vertices (flat shading)
            triangle_vertices[0].normal = face_normal;
            triangle_vertices[1].normal = face_normal;
            triangle_vertices[2].normal = face_normal;

            // Create triangle
            brh_triangle clip_space_triangle = {
                .vertices = { triangle_vertices[0], triangle_vertices[1], triangle_vertices[2] },
                .color = triangle_color,
            };

            // Temporary buffer for clipped triangles
            brh_triangle clipped_triangles[MAX_CLIPPED_TRIANGLES];
            int num_clipped_triangles = clip_triangle(&clip_space_triangle, clipped_triangles);

            // Process each clipped triangle
            for (int k = 0; k < num_clipped_triangles && triangle_count < buffer_capacity; k++) {
                brh_triangle triangle_to_render = clipped_triangles[k];

                // Perform perspective division and viewport transformation
                for (int l = 0; l < 3; l++) {
                    brh_vector4 clip_pos = triangle_to_render.vertices[l].position;

                    // Perspective division (clip space to NDC)
                    float inv_w = 1.0f / clip_pos.w;
                    brh_vector4 ndc_vertex;
                    ndc_vertex.x = clip_pos.x * inv_w;
                    ndc_vertex.y = clip_pos.y * inv_w;
                    ndc_vertex.z = clip_pos.z * inv_w;
                    ndc_vertex.w = clip_pos.w;

                    // Viewport transformation (NDC to screen space)
                    triangle_to_render.vertices[l].position.x = (ndc_vertex.x + 1.0f) * 0.5f * (float)get_window_width();
                    triangle_to_render.vertices[l].position.y = (1.0f - ndc_vertex.y) * 0.5f * (float)get_window_height();
                    triangle_to_render.vertices[l].position.z = ndc_vertex.z; // Now in NDC space [-1,1]
                    triangle_to_render.vertices[l].position.w = clip_pos.w;   // Preserve W for later use
                    triangle_to_render.vertices[l].inv_w = inv_w;             // Update inv_w for perspective correction
                }

                // Add to the triangle buffer
                triangle_buffer[triangle_count++] = triangle_to_render;
            }
        }
    }

    return triangle_count;
}

int get_scene_object_count(void)
{
    return scene_object_count;
}