#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "brh_mesh_manager.h"
#include "array.h"
#include "model_loader.h"

#define MAX_MESHES 32  // Maximum number of meshes that can be loaded simultaneously

typedef struct brh_mesh_handle_t {
    int id;            // Unique identifier for this mesh
    brh_mesh* mesh;    // Pointer to the actual mesh data
    bool is_valid;     // Whether this handle is valid
} brh_mesh_handle_t;

// Array of mesh handles
static brh_mesh_handle_t mesh_handles[MAX_MESHES];
static int next_mesh_id = 1;  // Start from 1, 0 can be reserved for invalid handles

bool initialize_mesh_system(void)
{
    // Initialize all mesh handles as invalid
    for (int i = 0; i < MAX_MESHES; i++) {
        mesh_handles[i].id = 0;
        mesh_handles[i].mesh = NULL;
        mesh_handles[i].is_valid = false;
    }
    next_mesh_id = 1;
    return true;
}

void cleanup_mesh_system(void)
{
    // Free all valid meshes
    for (int i = 0; i < MAX_MESHES; i++) {
        if (mesh_handles[i].is_valid && mesh_handles[i].mesh != NULL) {
            unload_mesh((brh_mesh_handle)&mesh_handles[i]);
        }
    }
}

brh_mesh_handle load_mesh(const char* file_path, bool is_right_handed)
{
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_MESHES; i++) {
        if (!mesh_handles[i].is_valid) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "Error: Maximum number of meshes (%d) reached\n", MAX_MESHES);
        return NULL;
    }

    // Allocate mesh structure
    brh_mesh* new_mesh = (brh_mesh*)malloc(sizeof(brh_mesh));
    if (!new_mesh) {
        fprintf(stderr, "Error: Failed to allocate memory for mesh\n");
        return NULL;
    }

    // Initialize mesh arrays
    new_mesh->vertices = NULL;
    new_mesh->faces = NULL;
    new_mesh->texcoords = NULL;
    new_mesh->normals = NULL;

    // Set default transform
    new_mesh->scale = (brh_vector3){ 1.0f, 1.0f, 1.0f };
    new_mesh->rotation = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    new_mesh->translation = (brh_vector3){ 0.0f, 0.0f, 0.0f };

    // Load mesh data from file
    bool loaded = load_obj(file_path, new_mesh, is_right_handed);
    if (!loaded) {
        fprintf(stderr, "Error: Failed to load mesh from file: %s\n", file_path);
        free(new_mesh);
        return NULL;
    }

    // Setup the handle
    mesh_handles[slot].id = next_mesh_id++;
    mesh_handles[slot].mesh = new_mesh;
    mesh_handles[slot].is_valid = true;

    return (brh_mesh_handle)&mesh_handles[slot];
}

void unload_mesh(brh_mesh_handle mesh_handle)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return;
    }

    brh_mesh_handle_t* handle = (brh_mesh_handle_t*)mesh_handle;
    brh_mesh* mesh = handle->mesh;

    // Free mesh resources
    if (mesh->vertices) {
        array_free(mesh->vertices);
        mesh->vertices = NULL;
    }

    if (mesh->faces) {
        array_free(mesh->faces);
        mesh->faces = NULL;
    }

    if (mesh->texcoords) {
        array_free(mesh->texcoords);
        mesh->texcoords = NULL;
    }

    if (mesh->normals) {
        array_free(mesh->normals);
        mesh->normals = NULL;
    }

    // Free mesh structure
    free(mesh);

    // Invalidate handle
    handle->mesh = NULL;
    handle->is_valid = false;
}

brh_mesh* get_mesh_data(brh_mesh_handle mesh_handle)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return NULL;
    }

    return ((brh_mesh_handle_t*)mesh_handle)->mesh;
}

void set_mesh_position(brh_mesh_handle mesh_handle, brh_vector3 position)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return;
    }

    ((brh_mesh_handle_t*)mesh_handle)->mesh->translation = position;
}

void set_mesh_rotation(brh_mesh_handle mesh_handle, brh_vector3 rotation)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return;
    }

    ((brh_mesh_handle_t*)mesh_handle)->mesh->rotation = rotation;
}

void set_mesh_scale(brh_mesh_handle mesh_handle, brh_vector3 scale)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return;
    }

    ((brh_mesh_handle_t*)mesh_handle)->mesh->scale = scale;
}

brh_vector3 get_mesh_position(brh_mesh_handle mesh_handle)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    return ((brh_mesh_handle_t*)mesh_handle)->mesh->translation;
}

brh_vector3 get_mesh_rotation(brh_mesh_handle mesh_handle)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    return ((brh_mesh_handle_t*)mesh_handle)->mesh->rotation;
}

brh_vector3 get_mesh_scale(brh_mesh_handle mesh_handle)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return (brh_vector3) { 1.0f, 1.0f, 1.0f };
    }

    return ((brh_mesh_handle_t*)mesh_handle)->mesh->scale;
}

int get_mesh_face_count(brh_mesh_handle mesh_handle)
{
    if (!mesh_handle || !((brh_mesh_handle_t*)mesh_handle)->is_valid) {
        return 0;
    }

    brh_mesh* mesh = ((brh_mesh_handle_t*)mesh_handle)->mesh;
    return array_length(mesh->faces);
}