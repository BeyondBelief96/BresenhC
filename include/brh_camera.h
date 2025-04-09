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
    brh_vector3 direction; // The direction the camera is facing.
} brh_look_at_camera;

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
brh_mat4 create_look_at_camera_matrix(brh_vector3 eyePosition, brh_vector3 target, brh_vector3 up);

extern brh_look_at_camera lookat_camera;




