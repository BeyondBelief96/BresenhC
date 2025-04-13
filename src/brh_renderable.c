#include <stdlib.h>
#include <stdio.h>
#include "brh_renderable.h"

#define MAX_RENDERABLES 32  // Maximum number of renderables that can be created simultaneously

typedef struct brh_renderable_handle_t {
    int id;                  // Unique identifier for this renderable
    brh_mesh_handle mesh;    // Handle to the mesh
    brh_texture_handle texture; // Handle to the texture (can be NULL)
    brh_vector3 position;    // Position in world space
    brh_vector3 rotation;    // Rotation in world space (in radians)
    brh_vector3 scale;       // Scale in world space
    brh_mat4 world_matrix;   // Cached world matrix
    bool is_valid;           // Whether this handle is valid
    bool needs_update;       // Whether the world matrix needs to be recalculated
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
    if (get_mesh_data(mesh_handle) == NULL) {
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

    // Setup the handle
    renderable_handles[slot].id = next_renderable_id++;
    renderable_handles[slot].mesh = mesh_handle;
    renderable_handles[slot].texture = texture_handle;
    renderable_handles[slot].position = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    renderable_handles[slot].rotation = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    renderable_handles[slot].scale = (brh_vector3){ 1.0f, 1.0f, 1.0f };
    renderable_handles[slot].world_matrix = mat4_identity();
    renderable_handles[slot].is_valid = true;
    renderable_handles[slot].needs_update = true;

    return (brh_renderable_handle)&renderable_handles[slot];
}

void destroy_renderable(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    // Just invalidate the handle (doesn't free mesh or texture)
    renderable_handle->is_valid = false;
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

void update_renderables(float delta_time)
{
    // Update all valid renderables
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        if (renderable_handles[i].is_valid && renderable_handles[i].needs_update) {
            // Calculate world matrix
            renderable_handles[i].world_matrix = mat4_create_world_matrix(
                renderable_handles[i].position,
                renderable_handles[i].rotation,
                renderable_handles[i].scale
            );
            renderable_handles[i].needs_update = false;
        }
    }
}
