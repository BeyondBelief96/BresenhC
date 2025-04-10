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
* 
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
brh_mat4 create_camera_look_at_matrix(brh_vector3 eyePosition, brh_vector3 target, brh_vector3 up);

extern brh_look_at_camera lookat_camera;
extern brh_fps_camera fps_camera;




