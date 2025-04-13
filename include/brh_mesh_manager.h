#pragma once

#include <stdbool.h>
#include "brh_mesh.h"

// Opaque handle to a mesh (hides implementation details)
typedef struct brh_mesh_handle_t* brh_mesh_handle;

/**
 * @brief Initialize the mesh management system
 *
 * @return true if initialization succeeded, false otherwise
 */
bool initialize_mesh_system(void);

/**
 * @brief Clean up the mesh management system and free all resources
 */
void cleanup_mesh_system(void);

/**
 * @brief Load a mesh from an OBJ file
 *
 * @param file_path Path to the OBJ file
 * @param is_right_handed Whether the mesh uses right-handed coordinates
 * @return A handle to the loaded mesh, or NULL if loading failed
 */
brh_mesh_handle load_mesh(const char* file_path, bool is_right_handed);

/**
 * @brief Unload a mesh and free its resources
 *
 * @param mesh_handle Handle to the mesh to unload
 */
void unload_mesh(brh_mesh_handle mesh_handle);

/**
 * @brief Get the mesh structure from a handle for rendering
 *
 * @param mesh_handle Handle to the mesh
 * @return Pointer to the mesh structure, or NULL if invalid handle
 */
brh_mesh* get_mesh_data(brh_mesh_handle mesh_handle);

/**
 * @brief Set the position of a mesh
 *
 * @param mesh_handle Handle to the mesh
 * @param position The new position
 */
void set_mesh_position(brh_mesh_handle mesh_handle, brh_vector3 position);

/**
 * @brief Set the rotation of a mesh
 *
 * @param mesh_handle Handle to the mesh
 * @param rotation The new rotation (in radians)
 */
void set_mesh_rotation(brh_mesh_handle mesh_handle, brh_vector3 rotation);

/**
 * @brief Set the scale of a mesh
 *
 * @param mesh_handle Handle to the mesh
 * @param scale The new scale
 */
void set_mesh_scale(brh_mesh_handle mesh_handle, brh_vector3 scale);

/**
 * @brief Get the position of a mesh
 *
 * @param mesh_handle Handle to the mesh
 * @return The current position
 */
brh_vector3 get_mesh_position(brh_mesh_handle mesh_handle);

/**
 * @brief Get the rotation of a mesh
 *
 * @param mesh_handle Handle to the mesh
 * @return The current rotation (in radians)
 */
brh_vector3 get_mesh_rotation(brh_mesh_handle mesh_handle);

/**
 * @brief Get the scale of a mesh
 *
 * @param mesh_handle Handle to the mesh
 * @return The current scale
 */
brh_vector3 get_mesh_scale(brh_mesh_handle mesh_handle);

/**
 * @brief Get the number of faces in a mesh
 *
 * @param mesh_handle Handle to the mesh
 * @return The number of faces, or 0 if invalid handle
 */
int get_mesh_face_count(brh_mesh_handle mesh_handle);