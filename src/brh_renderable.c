#include <stdlib.h>
#include <stdio.h>
#include "brh_renderable.h"
#include "brh_mesh_manager.h"
#include "brh_texture_manager.h"
#include "array.h"
#include "brh_light.h"
#include "brh_matrix.h"
#include "brh_clipping.h"
#include "brh_display.h"

#define MAX_RENDERABLES 32  // Maximum number of renderables that can be created simultaneously

static void update_renderable_triangles(brh_renderable_handle renderable_handle, brh_mat4 camera_matrix, brh_mat4 projection_matrix);

typedef struct brh_renderable_handle_t {
    int id;                  // Unique identifier for this renderable
    brh_mesh_handle mesh;    // Handle to the mesh
    brh_texture_handle texture; // Handle to the texture (can be NULL)
    brh_vector3 position;    // Position in world space
    brh_vector3 rotation;    // Rotation in world space (in radians)
    brh_vector3 scale;       // Scale in world space
    brh_mat4 world_matrix;   // Cached world matrix
    // Rendering data
    brh_triangle* triangles;      // Buffer of triangles to render
    int triangle_count;           // Number of triangles in the buffer
    int triangle_capacity;        // Capacity of the triangle buffer
    bool is_valid;           // Whether this handle is valid
    bool needs_update;       // Whether the world matrix needs to be recalculated
    bool owns_resources;     // Whether this renderable owns its mesh and texture
} brh_renderable_handle_t;

// Array of renderable handles
static brh_renderable_handle_t renderable_handles[MAX_RENDERABLES];
static int next_renderable_id = 1;  // Start from 1, 0 can be reserved for invalid handles

bool initialize_renderable_system(void)
{
    // Initialize all renderable handles as invalid
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        renderable_handles[i].id = 0;
        renderable_handles[i].mesh = NULL;
        renderable_handles[i].texture = NULL;
        renderable_handles[i].position = (brh_vector3){ 0.0f, 0.0f, 0.0f };
        renderable_handles[i].rotation = (brh_vector3){ 0.0f, 0.0f, 0.0f };
        renderable_handles[i].scale = (brh_vector3){ 1.0f, 1.0f, 1.0f };
        renderable_handles[i].world_matrix = mat4_identity();
        renderable_handles[i].is_valid = false;
        renderable_handles[i].needs_update = true;
        renderable_handles[i].owns_resources = false;
    }
    next_renderable_id = 1;
    return true;
}

void cleanup_renderable_system(void)
{
    // Free all valid renderables
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        if (renderable_handles[i].is_valid) {
            destroy_renderable((brh_renderable_handle)&renderable_handles[i]);
        }
    }
}

brh_renderable_handle create_renderable(brh_mesh_handle mesh_handle, brh_texture_handle texture_handle)
{
    // Check if mesh handle is valid
    if (mesh_handle && get_mesh_data(mesh_handle) == NULL) {
        fprintf(stderr, "Error: Invalid mesh handle\n");
        return NULL;
    }

    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        if (!renderable_handles[i].is_valid) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "Error: Maximum number of renderables (%d) reached\n", MAX_RENDERABLES);
        return NULL;
    }

    // Get mesh face count to allocate triangle buffer
    int face_count = 0;
    if (mesh_handle) {
        brh_mesh* mesh_data = get_mesh_data(mesh_handle);
        face_count = array_length(mesh_data->faces);
    }

    // Allocate triangle buffer
    brh_triangle* triangles = NULL;
    if (face_count > 0) {
        triangles = (brh_triangle*)malloc(sizeof(brh_triangle) * face_count);
        if (!triangles) {
            fprintf(stderr, "Error: Failed to allocate triangle buffer for renderable\n");
            return NULL;
        }
    }

    // Setup the handle
    renderable_handles[slot].id = next_renderable_id++;
    renderable_handles[slot].mesh = mesh_handle;
    renderable_handles[slot].texture = texture_handle;
    renderable_handles[slot].position = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    renderable_handles[slot].rotation = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    renderable_handles[slot].scale = (brh_vector3){ 1.0f, 1.0f, 1.0f };
    renderable_handles[slot].world_matrix = mat4_identity();
    renderable_handles[slot].triangles = triangles;
    renderable_handles[slot].triangle_count = 0;
    renderable_handles[slot].triangle_capacity = face_count;
    renderable_handles[slot].is_valid = true;
    renderable_handles[slot].needs_update = true;
    renderable_handles[slot].owns_resources = false;

    return (brh_renderable_handle)&renderable_handles[slot];
}

brh_renderable_handle create_renderable_from_files(const char* mesh_file, const char* texture_file)
{
    // Load mesh
    brh_mesh_handle mesh_handle = load_mesh(mesh_file, true);
    if (!mesh_handle) {
        fprintf(stderr, "Error: Failed to load mesh: %s\n", mesh_file);
        return NULL;
    }

    // Load texture if provided
    brh_texture_handle texture_handle = NULL;
    if (texture_file != NULL) {
        texture_handle = load_texture(texture_file);
        if (!texture_handle) {
            fprintf(stderr, "Error: Failed to load texture: %s\n", texture_file);
            unload_mesh(mesh_handle);
            return NULL;
        }
    }

    // Create renderable
    brh_renderable_handle renderable = create_renderable(mesh_handle, texture_handle);
    if (!renderable) {
        if (texture_handle) unload_texture(texture_handle);
        unload_mesh(mesh_handle);
        return NULL;
    }

    // Mark that this renderable owns its resources
    ((brh_renderable_handle_t*)renderable)->owns_resources = true;

    return renderable;
}

void destroy_renderable(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;

    // Free triangle buffer
    if (handle->triangles) {
        free(handle->triangles);
        handle->triangles = NULL;
        handle->triangle_count = 0;
        handle->triangle_capacity = 0;
    }

    // If this renderable owns its resources, unload them
    if (handle->owns_resources) {
        if (handle->texture) {
            unload_texture(handle->texture);
        }
        if (handle->mesh) {
            unload_mesh(handle->mesh);
        }
    }

    // Invalidate the handle
    handle->is_valid = false;
    handle->mesh = NULL;
    handle->texture = NULL;
}

void set_renderable_position(brh_renderable_handle renderable_handle, brh_vector3 position)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;
    handle->position = position;
    handle->needs_update = true;
}

void set_renderable_rotation(brh_renderable_handle renderable_handle, brh_vector3 rotation)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;
    handle->rotation = rotation;
    handle->needs_update = true;
}

void set_renderable_scale(brh_renderable_handle renderable_handle, brh_vector3 scale)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;
    handle->scale = scale;
    handle->needs_update = true;
}

brh_vector3 get_renderable_position(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    return ((brh_renderable_handle_t*)renderable_handle)->position;
}

brh_vector3 get_renderable_rotation(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    return ((brh_renderable_handle_t*)renderable_handle)->rotation;
}

brh_vector3 get_renderable_scale(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return (brh_vector3) { 1.0f, 1.0f, 1.0f };
    }

    return ((brh_renderable_handle_t*)renderable_handle)->scale;
}

brh_mat4 get_renderable_world_matrix(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return mat4_identity();
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;

    // Update world matrix if needed
    if (handle->needs_update) {
        handle->world_matrix = mat4_create_world_matrix(
            handle->position,
            handle->rotation,
            handle->scale
        );
        handle->needs_update = false;
    }

    return handle->world_matrix;
}

brh_mesh_handle get_renderable_mesh(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return NULL;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->mesh;
}

brh_texture_handle get_renderable_texture(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return NULL;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->texture;
}

static void update_renderable_triangles(brh_renderable_handle renderable_handle, brh_mat4 camera_matrix, brh_mat4 projection_matrix)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;

    // Skip if no mesh or no triangle buffer
    if (!handle->mesh || !handle->triangles || handle->triangle_capacity <= 0) {
        return;
    }

    // Get mesh data
    brh_mesh* mesh_data = get_mesh_data(handle->mesh);
    if (!mesh_data) {
        return;
    }

    // Reset triangle count
    handle->triangle_count = 0;

    // Get world matrix
    brh_mat4 world_matrix = get_renderable_world_matrix(renderable_handle);

    // Process mesh faces
    int num_faces = array_length(mesh_data->faces);
    int num_texcoords = array_length(mesh_data->texcoords);

    // Temporary buffer for clipped triangles
    brh_triangle clipped_triangles[MAX_CLIPPED_TRIANGLES];

    for (int i = 0; i < num_faces && handle->triangle_count < handle->triangle_capacity; i++) {
        brh_face face = mesh_data->faces[i];
        brh_vector3 face_vertices_model[3];
        brh_vector4 face_vertices_world[3];
        brh_vector4 face_vertices_camera[3];
        brh_vertex triangle_vertices[3];
        brh_texel face_texels[3] = { {0, 0}, {0, 0}, {0, 0} };

        // Get vertices and texcoords
        face_vertices_model[0] = mesh_data->vertices[face.a];
        face_vertices_model[1] = mesh_data->vertices[face.b];
        face_vertices_model[2] = mesh_data->vertices[face.c];

        if (num_texcoords > 0) {
            if (face.a_vt < num_texcoords) face_texels[0] = mesh_data->texcoords[face.a_vt];
            if (face.b_vt < num_texcoords) face_texels[1] = mesh_data->texcoords[face.b_vt];
            if (face.c_vt < num_texcoords) face_texels[2] = mesh_data->texcoords[face.c_vt];
        }

        // Transform vertices
        for (int j = 0; j < 3; j++) {
            brh_vector4 world_vertex = vec4_from_vec3(face_vertices_model[j]);
            mat4_mul_vec4_ref(&world_matrix, &world_vertex);
            face_vertices_world[j] = world_vertex;

            brh_vector4 camera_vertex = face_vertices_world[j];
            mat4_mul_vec4_ref(&camera_matrix, &camera_vertex);
            face_vertices_camera[j] = camera_vertex;

            brh_vector4 clip_vertex = mat4_mul_vec4(&projection_matrix, camera_vertex);
            float original_w = clip_vertex.w;
            triangle_vertices[j].inv_w = 1.0f / original_w;
            triangle_vertices[j].position = clip_vertex;
            triangle_vertices[j].texel = face_texels[j];
        }

        // Backface culling
        if (get_cull_method() == CULL_BACKFACE) {
            brh_vector3 vecA_camera = vec3_from_vec4(face_vertices_camera[0]);
            brh_vector3 vecB_camera = vec3_from_vec4(face_vertices_camera[1]);
            brh_vector3 vecC_camera = vec3_from_vec4(face_vertices_camera[2]);
            brh_vector3 face_normal = get_face_normal(vecA_camera, vecB_camera, vecC_camera);

            brh_vector3 origin = { 0.0f, 0.0f, 0.0f };
            brh_vector3 view_vector = vec3_subtract(origin, vecA_camera);
            float angle_dot_product = vec3_dot(face_normal, view_vector);

            if (angle_dot_product < 0) {
                continue;
            }
        }

        // Calculate face normal for lighting
        brh_vector3 vecA_world = vec3_from_vec4(face_vertices_world[0]);
        brh_vector3 vecB_world = vec3_from_vec4(face_vertices_world[1]);
        brh_vector3 vecC_world = vec3_from_vec4(face_vertices_world[2]);
        brh_vector3 face_normal = get_face_normal(vecA_world, vecB_world, vecC_world);

        uint32_t triangle_color = calculate_flat_shading_color(face_normal, face.color);
        triangle_vertices[0].normal = face_normal;
        triangle_vertices[1].normal = face_normal;
        triangle_vertices[2].normal = face_normal;

        brh_triangle clip_space_triangle = {
            .vertices = { triangle_vertices[0], triangle_vertices[1], triangle_vertices[2] },
            .color = triangle_color,
        };

        // Clip triangle
        int num_clipped_triangles = clip_triangle(&clip_space_triangle, clipped_triangles);

        // Process clipped triangles
        for (int j = 0; j < num_clipped_triangles && handle->triangle_count < handle->triangle_capacity; j++) {
            brh_triangle triangle_to_render = clipped_triangles[j];

            // Perform perspective division and viewport transformation
            for (int k = 0; k < 3; k++) {
                brh_vector4 clip_pos = triangle_to_render.vertices[k].position;
                float inv_w = 1.0f / clip_pos.w;
                brh_vector4 ndc_vertex;
                ndc_vertex.x = clip_pos.x * inv_w;
                ndc_vertex.y = clip_pos.y * inv_w;
                ndc_vertex.z = clip_pos.z * inv_w;
                ndc_vertex.w = clip_pos.w;

                triangle_to_render.vertices[k].position.x = (ndc_vertex.x + 1.0f) * 0.5f * (float)get_window_width();
                triangle_to_render.vertices[k].position.y = (1.0f - ndc_vertex.y) * 0.5f * (float)get_window_height();
                triangle_to_render.vertices[k].position.z = ndc_vertex.z;
                triangle_to_render.vertices[k].position.w = clip_pos.w;
                triangle_to_render.vertices[k].inv_w = inv_w;
            }

            // Add triangle to buffer
            handle->triangles[handle->triangle_count++] = triangle_to_render;
        }


        if (handle->triangle_count >= handle->triangle_capacity) {
            fprintf(stderr, "Warning: Triangle buffer filled to capacity (%d triangles)\n", handle->triangle_capacity);
            break;
        }
    }
}

brh_triangle* get_renderable_triangles(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return NULL;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->triangles;
}

int get_renderable_triangle_count(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return 0;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->triangle_count;
}

void update_renderables(float delta_time, brh_mat4 camera_matrix, brh_mat4 projection_matrix)
{
    // Update all valid renderables
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        if (renderable_handles[i].is_valid) {
            // Update world matrix if needed
            if (renderable_handles[i].needs_update) {
                renderable_handles[i].world_matrix = mat4_create_world_matrix(
                    renderable_handles[i].position,
                    renderable_handles[i].rotation,
                    renderable_handles[i].scale
                );
                renderable_handles[i].needs_update = false;
            }

            // Update triangles
            update_renderable_triangles((brh_renderable_handle)&renderable_handles[i],
                camera_matrix,
                projection_matrix);
        }
    }
}