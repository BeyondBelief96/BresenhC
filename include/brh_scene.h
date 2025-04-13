#pragma once

#include "brh_renderable.h"
#include "brh_camera.h"
#include "brh_triangle.h"

/**
 * @brief Initialize the scene system
 *
 * @return true if initialization succeeded, false otherwise
 */
bool initialize_scene_system(void);

/**
 * @brief Clean up the scene system and free all resources
 */
void cleanup_scene_system(void);

/**
 * @brief Add a renderable object to the scene
 *
 * @param renderable_handle Handle to the renderable object
 * @return true if the object was added successfully, false otherwise
 */
bool add_to_scene(brh_renderable_handle renderable_handle);

/**
 * @brief Remove a renderable object from the scene
 *
 * @param renderable_handle Handle to the renderable object
 */
void remove_from_scene(brh_renderable_handle renderable_handle);

/**
 * @brief Set the active camera for the scene
 *
 * @param camera_handle Handle to the camera
 */
void set_scene_camera(brh_mouse_camera* camera);

/**
 * @brief Update all objects in the scene
 *
 * @param delta_time Time elapsed since last frame
 */
void update_scene(float delta_time);

/**
 * @brief Render the scene
 *
 * @param triangle_buffer Buffer to store the triangles to render
 * @param buffer_capacity Maximum number of triangles that can fit in the buffer
 * @return Number of triangles added to the buffer
 */
int render_scene(brh_triangle* triangle_buffer, int buffer_capacity);

/**
 * @brief Get the number of objects in the scene
 *
 * @return The number of objects
 */
int get_scene_object_count(void);