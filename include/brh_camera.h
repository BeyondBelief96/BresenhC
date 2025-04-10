#pragma once

#include "brh_vector.h"
#include "brh_matrix.h"

/**
 * @brief Represents a simple look-at camera model for a 3D renderer.
 *
 * This structure defines a camera with a position and a direction vector.
 * The camera can be used to render 3D objects in a 3D space and can look at
 * a specific point using the `mat4_create_look_at_matrix` function.
 */
typedef struct {
    brh_vector3 position;  // The position of the camera in 3D space.
    brh_vector3 target; // The target the camera is looking at.
} brh_look_at_camera;

typedef struct {
	brh_vector3 position; // The position of the camera in 3D space.
	brh_vector3 direction;   // The direction the camera is facing.
	brh_vector3 forward_velocity; // The forward velocity of the camera.
	float yaw_angle; // The yaw angle of the camera.
} brh_fps_camera;

/*
* @brief Represents a mouse-controlled camera for 3D rendering.
* This structure defines a camera that can be controlled using mouse movements.
* It includes the camera's position, direction, and rotation angles.
* The camera can be used to render 3D objects in a 3D space and can be manipulated
* using mouse input to change its orientation and position.
*/
typedef struct {
	brh_vector3 position; // The position of the camera in 3D space.
	brh_vector3 direction; // The direction the camera is facing.
	float yaw_angle; // The yaw angle of the camera.
	float pitch_angle; // The pitch angle of the camera.
	float speed; // The speed of the camera movement.
	float sensitivity; // The sensitivity of the mouse movement.
} brh_mouse_camera;

extern brh_look_at_camera lookat_camera;
extern brh_fps_camera fps_camera;
extern brh_mouse_camera mouse_camera;
extern float frustum_fov_y;
extern float near_plane;
extern float far_plane;

/**
 * @brief Creates a view matrix for a camera that looks at a specific point in 3D space.
 *
 * This function generates a view matrix based on the camera's position, the target point
 * it is looking at, and an up vector that defines the camera's orientation.
 * Note this function assumes a left-handed coordinate system.
 *
 * @param eyePosition The position of the camera (eye) in 3D space.
 * @param target The point in 3D space the camera is looking at.
 * @param up The up vector defining the camera's orientation.
 * @return A 4x4 view matrix representing the camera's transformation.
 */
brh_mat4 create_camera_matrix(brh_vector3 eyePosition, brh_vector3 target, brh_vector3 up);

/**
 * @brief Updates a mouse-controlled camera based on mouse movement.
 *
 * @param camera Pointer to the mouse camera to update.
 * @param mouse_x_rel Relative mouse X movement since last frame.
 * @param mouse_y_rel Relative mouse Y movement since last frame.
 * @param delta_time Time elapsed since last frame in seconds.
 */
void update_mouse_camera_view(brh_mouse_camera* camera, int mouse_x_rel, int mouse_y_rel, float delta_time);

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
 * @brief Gets the view matrix for a mouse-controlled camera.
 *
 * @param camera Pointer to the mouse camera.
 * @return The view matrix for the camera.
 */
brh_mat4 get_mouse_camera_view_matrix(brh_mouse_camera* camera);




