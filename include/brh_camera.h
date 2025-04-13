#pragma once
#include <stdbool.h>
#include "brh_vector.h"
#include "brh_matrix.h"

/**
 * @brief Represents a simple look-at camera model for a 3D renderer.
 */
typedef struct brh_look_at_camera brh_look_at_camera;

/**
 * @brief Represents an FPS-style camera with velocity.
 */
typedef struct brh_fps_camera brh_fps_camera;

/**
 * @brief Represents a mouse-controlled camera for 3D rendering.
 */
typedef struct brh_mouse_camera brh_mouse_camera;

/**
 * @brief Create and configure a look-at camera.
 *
 * @param position The position of the camera.
 * @param target The point the camera is looking at.
 * @return A new look-at camera with the specified parameters.
 */
brh_look_at_camera* create_look_at_camera(brh_vector3 position, brh_vector3 target);

/**
 * @brief Create and configure a mouse-controlled camera.
 *
 * @param position The initial position of the camera.
 * @param direction The initial direction the camera is facing.
 * @param speed Movement speed of the camera.
 * @param sensitivity Mouse sensitivity.
 * @return A new mouse-controlled camera with the specified parameters.
 */
brh_mouse_camera* create_mouse_camera(brh_vector3 position, brh_vector3 direction, float speed, float sensitivity);

/**
 * @brief Free resources used by a look-at camera.
 *
 * @param camera The camera to destroy.
 */
void destroy_look_at_camera(brh_look_at_camera* camera);

/**
 * @brief Free resources used by a mouse camera.
 *
 * @param camera The camera to destroy.
 */
void destroy_mouse_camera(brh_mouse_camera* camera);

/**
 * @brief Creates a view matrix for a camera that looks at a specific point in 3D space.
 *
 * @param camera The look-at camera to use.
 * @return A 4x4 view matrix representing the camera's transformation.
 */
brh_mat4 get_look_at_camera_view_matrix(const brh_look_at_camera* camera);

/**
 * @brief Gets the view matrix for a mouse-controlled camera.
 *
 * @param camera Pointer to the mouse camera.
 * @return The view matrix for the camera.
 */
brh_mat4 get_mouse_camera_view_matrix(const brh_mouse_camera* camera);

/**
 * @brief Updates a mouse-controlled camera based on mouse movement.
 *
 * @param camera Pointer to the mouse camera to update.
 * @param mouse_x_rel Relative mouse X movement since last frame.
 * @param mouse_y_rel Relative mouse Y movement since last frame.
 */
void update_mouse_camera_view(brh_mouse_camera* camera, int mouse_x_rel, int mouse_y_rel);

/**
 * @brief Moves a mouse-controlled camera based on keyboard input.
 *
 * @param camera Pointer to the mouse camera to move.
 * @param forward Forward/backward movement direction (-1, 0, 1).
 * @param right Right/left movement direction (-1, 0, 1).
 * @param up Up/down movement direction (-1, 0, 1).
 * @param delta_time Time elapsed since last frame in seconds.
 */
void move_mouse_camera(brh_mouse_camera* camera, int forward, int right, int up, float delta_time);

/**
 * @brief Set the frustum configuration parameters.
 *
 * @param fov_y Field of view in degrees.
 * @param near Near clipping plane distance.
 * @param far Far clipping plane distance.
 */
void set_frustum_parameters(float fov_y, float near, float far);

/**
 * @brief Get the current field of view.
 *
 * @return The current field of view in degrees.
 */
float get_frustum_fov_y(void);

/**
 * @brief Get the current near plane distance.
 *
 * @return The distance to the near clipping plane.
 */
float get_frustum_near_plane(void);

/**
 * @brief Get the current far plane distance.
 *
 * @return The distance to the far clipping plane.
 */
float get_frustum_far_plane(void);

/**
 * @brief Get the position of a look-at camera.
 *
 * @param camera The camera to get the position of.
 * @return The position as a 3D vector.
 */
brh_vector3 get_look_at_camera_position(const brh_look_at_camera* camera);

/**
 * @brief Set the position of a look-at camera.
 *
 * @param camera The camera to update.
 * @param position The new position.
 */
void set_look_at_camera_position(brh_look_at_camera* camera, brh_vector3 position);

/**
 * @brief Get the target of a look-at camera.
 *
 * @param camera The camera to get the target of.
 * @return The target as a 3D vector.
 */
brh_vector3 get_look_at_camera_target(const brh_look_at_camera* camera);

/**
 * @brief Set the target of a look-at camera.
 *
 * @param camera The camera to update.
 * @param target The new target.
 */
void set_look_at_camera_target(brh_look_at_camera* camera, brh_vector3 target);

/**
 * @brief Get the position of a mouse camera.
 *
 * @param camera The camera to get the position of.
 * @return The position as a 3D vector.
 */
brh_vector3 get_mouse_camera_position(const brh_mouse_camera* camera);

/**
 * @brief Set the position of a mouse camera.
 *
 * @param camera The camera to update.
 * @param position The new position.
 */
void set_mouse_camera_position(brh_mouse_camera* camera, brh_vector3 position);

/**
 * @brief Get the direction of a mouse camera.
 *
 * @param camera The camera to get the direction of.
 * @return The direction as a 3D vector.
 */
brh_vector3 get_mouse_camera_direction(const brh_mouse_camera* camera);

/**
 * @brief Set the direction of a mouse camera directly.
 *
 * @param camera The camera to update.
 * @param direction The new direction (will be normalized).
 */
void set_mouse_camera_direction(brh_mouse_camera* camera, brh_vector3 direction);

/**
 * @brief Get the rotation (yaw, pitch) of a mouse camera.
 *
 * @param camera The camera to get the rotation of.
 * @param yaw_angle Pointer to store the yaw angle (in radians).
 * @param pitch_angle Pointer to store the pitch angle (in radians).
 */
void get_mouse_camera_rotation(const brh_mouse_camera* camera, float* yaw_angle, float* pitch_angle);

/**
 * @brief Set the rotation of a mouse camera.
 *
 * @param camera The camera to update.
 * @param yaw_angle The new yaw angle (in radians).
 * @param pitch_angle The new pitch angle (in radians, will be clamped).
 */
void set_mouse_camera_rotation(brh_mouse_camera* camera, float yaw_angle, float pitch_angle);

/**
 * @brief Set the mouse camera's movement speed.
 *
 * @param camera The camera to update.
 * @param speed The new movement speed.
 */
void set_mouse_camera_speed(brh_mouse_camera* camera, float speed);

/**
 * @brief Get the mouse camera's movement speed.
 *
 * @param camera The camera to query.
 * @return The current movement speed.
 */
float get_mouse_camera_speed(const brh_mouse_camera* camera);

/**
 * @brief Set the mouse camera's sensitivity.
 *
 * @param camera The camera to update.
 * @param sensitivity The new mouse sensitivity.
 */
void set_mouse_camera_sensitivity(brh_mouse_camera* camera, float sensitivity);

/**
 * @brief Get the mouse camera's sensitivity.
 *
 * @param camera The camera to query.
 * @return The current mouse sensitivity.
 */
float get_mouse_camera_sensitivity(const brh_mouse_camera* camera);