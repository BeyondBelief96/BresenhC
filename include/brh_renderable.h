#pragma once

#include "brh_mesh_manager.h"
#include "brh_texture_manager.h"
#include "brh_matrix.h"
#include "brh_camera.h"

// Opaque handle to a renderable object (hides implementation details)
typedef struct brh_renderable_handle_t* brh_renderable_handle;

/**
 * @brief Initialize the renderable object system
 *
 * @return true if initialization succeeded, false otherwise
 */
bool initialize_renderable_system(void);

/**
 * @brief Clean up the renderable object system and free all resources
 */
void cleanup_renderable_system(void);

/**
 * @brief Create a renderable object from a mesh and texture
 *
 * @param mesh_handle Handle to the mesh
 * @param texture_handle Handle to the texture (can be NULL for untextured objects)
 * @return A handle to the renderable object, or NULL if creation failed
 */
brh_renderable_handle create_renderable(brh_mesh_handle mesh_handle, brh_texture_handle texture_handle); \

/*
* @brief Create a renderable object from mesh and texture files
*
* @param mesh_file Path to the mesh file
* @param texture_file Path to the texture file (can be NULL for untextured objects)
* @return A handle to the renderable object, or NULL if creation failed
*/
brh_renderable_handle create_renderable_from_files(const char* mesh_file, const char* texture_file);

/**
 * @brief Destroy a renderable object (does not free the mesh or texture)
 *
 * @param renderable_handle Handle to the renderable object
 */
void destroy_renderable(brh_renderable_handle renderable_handle);

/**
 * @brief Set the position of a renderable object
 *
 * @param renderable_handle Handle to the renderable object
 * @param position The new position
 */
void set_renderable_position(brh_renderable_handle renderable_handle, brh_vector3 position);

/**
 * @brief Set the rotation of a renderable object
 *
 * @param renderable_handle Handle to the renderable object
 * @param rotation The new rotation (in radians)
 */
void set_renderable_rotation(brh_renderable_handle renderable_handle, brh_vector3 rotation);

/**
 * @brief Set the scale of a renderable object
 *
 * @param renderable_handle Handle to the renderable object
 * @param scale The new scale
 */
void set_renderable_scale(brh_renderable_handle renderable_handle, brh_vector3 scale);

/**
 * @brief Get the world matrix for a renderable object
 *
 * @param renderable_handle Handle to the renderable object
 * @return The world matrix
 */
brh_mat4 get_renderable_world_matrix(brh_renderable_handle renderable_handle);

/**
 * @brief Get the mesh handle for a renderable object
 *
 * @param renderable_handle Handle to the renderable object
 * @return The mesh handle
 */
brh_mesh_handle get_renderable_mesh(brh_renderable_handle renderable_handle);

/**
 * @brief Get the texture handle for a renderable object
 *
 * @param renderable_handle Handle to the renderable object
 * @return The texture handle, or NULL if untextured
 */
brh_texture_handle get_renderable_texture(brh_renderable_handle renderable_handle);

/*
* * @brief Get the triangles to render for a renderable object
* 
* @param renderable_handle Handle to the renderable object
* @return Pointer to the array of triangles
*/
brh_triangle* get_renderable_triangles(brh_renderable_handle renderable_handle);

/*
* * @brief Get the number of triangles to render for a renderable object
* 
* @param renderable_handle Handle to the renderable object
* 
* @return The number of triangles
*/
int get_renderable_triangle_count(brh_renderable_handle renderable_handle);

/**
 * @brief Update all renderable objects (called once per frame)
 *
 * @param delta_time Time elapsed since last frame
 * @param camera_matrix The camera view matrix
 * @param projection_matrix The projection matrix
 */
void update_renderables(float delta_time, brh_mat4 camera_matrix, brh_mat4 projection_matrix,  brh_mouse_camera* camera);