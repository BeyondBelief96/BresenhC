#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "math_utils.h"
#include "brh_camera.h"

struct brh_look_at_camera {
    brh_vector3 position;  // The position of the camera in 3D space.
    brh_vector3 target;    // The target the camera is looking at.
};

struct brh_fps_camera {
    brh_vector3 position;          // The position of the camera in 3D space.
    brh_vector3 direction;         // The direction the camera is facing.
    brh_vector3 forward_velocity;  // The forward velocity of the camera.
    float yaw_angle;               // The yaw angle of the camera.
};

struct brh_mouse_camera {
    brh_vector3 position;    // The position of the camera in 3D space.
    brh_vector3 direction;   // The direction the camera is facing.
    float yaw_angle;         // The yaw angle of the camera.
    float pitch_angle;       // The pitch angle of the camera.
    float speed;             // The speed of the camera movement.
    float sensitivity;       // The sensitivity of the mouse movement.
};

// Module-level globals
static float frustum_fov_y = 60.0f;
static float near_plane = 1.0f;
static float far_plane = 100.0f;

// Static function to create a camera view matrix
static brh_mat4 create_camera_matrix(brh_vector3 eyePosition, brh_vector3 target, brh_vector3 up)
{
    brh_vector3 forward = vec3_unit_vector(vec3_subtract(target, eyePosition));
    brh_vector3 right = vec3_unit_vector(vec3_cross(up, forward));
    brh_vector3 up_normalized = vec3_unit_vector(vec3_cross(forward, right));
    brh_mat4 view_matrix = mat4_identity();
    view_matrix.m[0][0] = right.x;
    view_matrix.m[1][0] = up_normalized.x;
    view_matrix.m[2][0] = forward.x;
    view_matrix.m[0][1] = right.y;
    view_matrix.m[1][1] = up_normalized.y;
    view_matrix.m[2][1] = forward.y;
    view_matrix.m[0][2] = right.z;
    view_matrix.m[1][2] = up_normalized.z;
    view_matrix.m[2][2] = forward.z;
    view_matrix.m[0][3] = -vec3_dot(right, eyePosition);
    view_matrix.m[1][3] = -vec3_dot(up_normalized, eyePosition);
    view_matrix.m[2][3] = -vec3_dot(forward, eyePosition);
    return view_matrix;
}

brh_look_at_camera* create_look_at_camera(brh_vector3 position, brh_vector3 target)
{
    brh_look_at_camera* camera = (brh_look_at_camera*)malloc(sizeof(brh_look_at_camera));
    if (camera) {
        camera->position = position;
        camera->target = target;
    }
    return camera;
}

brh_mouse_camera* create_mouse_camera(brh_vector3 position, brh_vector3 direction, float speed, float sensitivity)
{
    brh_mouse_camera* camera = (brh_mouse_camera*)malloc(sizeof(brh_mouse_camera));
    if (camera) {
        camera->position = position;
        camera->direction = direction;

        // Normalize direction
        vec3_normalize(&camera->direction);

        // Calculate initial yaw and pitch from direction
        camera->yaw_angle = atan2f(direction.x, direction.z);
        camera->pitch_angle = asinf(direction.y);

        camera->speed = speed;
        camera->sensitivity = sensitivity;
    }
    return camera;
}

void destroy_look_at_camera(brh_look_at_camera* camera)
{
    if (camera) {
        free(camera);
    }
}

void destroy_mouse_camera(brh_mouse_camera* camera)
{
    if (camera) {
        free(camera);
    }
}

brh_mat4 get_look_at_camera_view_matrix(const brh_look_at_camera* camera)
{
    if (!camera) {
        // Return identity matrix if null
        return mat4_identity();
    }

    brh_vector3 up = { 0.0f, 1.0f, 0.0f };
    return create_camera_matrix(camera->position, camera->target, up);
}

brh_mat4 get_mouse_camera_view_matrix(const brh_mouse_camera* camera)
{
    if (!camera) {
        // Return identity matrix if null
        return mat4_identity();
    }

    brh_vector3 target = vec3_add(camera->position, camera->direction);
    brh_vector3 up = { 0.0f, 1.0f, 0.0f };
    return create_camera_matrix(camera->position, target, up);
}

void update_mouse_camera_view(brh_mouse_camera* camera, int mouse_x_rel, int mouse_y_rel)
{
    if (!camera) return;

    // Update yaw and pitch based on mouse movement
    camera->yaw_angle += (float)mouse_x_rel * camera->sensitivity;
    camera->pitch_angle -= (float)mouse_y_rel * camera->sensitivity;

    // Clamp pitch to avoid gimbal lock
    if (camera->pitch_angle > degrees_to_radians(89.0f)) {
        camera->pitch_angle = degrees_to_radians(89.0f);
    }
    if (camera->pitch_angle < degrees_to_radians(-89.0f)) {
        camera->pitch_angle = degrees_to_radians(-89.0f);
    }

    // Calculate direction vector from yaw and pitch
    camera->direction.x = sinf(camera->yaw_angle) * cosf(camera->pitch_angle);
    camera->direction.y = sinf(camera->pitch_angle);
    camera->direction.z = cosf(camera->yaw_angle) * cosf(camera->pitch_angle);
    vec3_normalize(&camera->direction);
}

void move_mouse_camera(brh_mouse_camera* camera, int forward, int right, int up, float delta_time)
{
    if (!camera) return;

    float velocity = camera->speed * delta_time;

    // For left-handed coordinate system, the cross product is camera->direction × world_up
    brh_vector3 world_up = { 0.0f, 1.0f, 0.0f };
    brh_vector3 right_vec = vec3_cross(world_up, camera->direction);
    vec3_normalize(&right_vec);
    brh_vector3 up_vec = vec3_cross(camera->direction, right_vec);
    vec3_normalize(&up_vec);

    if (forward != 0) {
        brh_vector3 forward_movement = vec3_scale(camera->direction, (float)forward * velocity);
        camera->position = vec3_add(camera->position, forward_movement);
    }

    if (right != 0) {
        brh_vector3 right_movement = vec3_scale(right_vec, (float)right * velocity);
        camera->position = vec3_add(camera->position, right_movement);
    }

    if (up != 0) {
        brh_vector3 up_movement = vec3_scale(up_vec, (float)up * velocity);
        camera->position = vec3_add(camera->position, up_movement);
    }
}

void set_frustum_parameters(float fov_y, float near, float far)
{
    frustum_fov_y = fov_y;
    near_plane = near;
    far_plane = far;
}

float get_frustum_fov_y(void)
{
    return frustum_fov_y;
}

float get_frustum_near_plane(void)
{
    return near_plane;
}

float get_frustum_far_plane(void)
{
    return far_plane;
}

brh_vector3 get_look_at_camera_position(const brh_look_at_camera* camera)
{
    return camera ? camera->position : (brh_vector3) { 0.0f, 0.0f, 0.0f };
}

void set_look_at_camera_position(brh_look_at_camera* camera, brh_vector3 position)
{
    if (camera) {
        camera->position = position;
    }
}

brh_vector3 get_look_at_camera_target(const brh_look_at_camera* camera)
{
    return camera ? camera->target : (brh_vector3) { 0.0f, 0.0f, 1.0f };
}

void set_look_at_camera_target(brh_look_at_camera* camera, brh_vector3 target)
{
    if (camera) {
        camera->target = target;
    }
}

brh_vector3 get_mouse_camera_position(const brh_mouse_camera* camera)
{
    return camera ? camera->position : (brh_vector3) { 0.0f, 0.0f, 0.0f };
}

void set_mouse_camera_position(brh_mouse_camera* camera, brh_vector3 position)
{
    if (camera) {
        camera->position = position;
    }
}

brh_vector3 get_mouse_camera_direction(const brh_mouse_camera* camera)
{
    return camera ? camera->direction : (brh_vector3) { 0.0f, 0.0f, 1.0f };
}

void set_mouse_camera_direction(brh_mouse_camera* camera, brh_vector3 direction)
{
    if (camera) {
        camera->direction = direction;
        vec3_normalize(&camera->direction);

        // Update angles
        camera->yaw_angle = atan2f(direction.x, direction.z);
        camera->pitch_angle = asinf(direction.y);
    }
}

void get_mouse_camera_rotation(const brh_mouse_camera* camera, float* yaw_angle, float* pitch_angle)
{
    if (camera && yaw_angle && pitch_angle) {
        *yaw_angle = camera->yaw_angle;
        *pitch_angle = camera->pitch_angle;
    }
}

void set_mouse_camera_rotation(brh_mouse_camera* camera, float yaw_angle, float pitch_angle)
{
    if (camera) {
        camera->yaw_angle = yaw_angle;

        // Clamp pitch
        if (pitch_angle > degrees_to_radians(89.0f)) {
            camera->pitch_angle = degrees_to_radians(89.0f);
        }
        else if (pitch_angle < degrees_to_radians(-89.0f)) {
            camera->pitch_angle = degrees_to_radians(-89.0f);
        }
        else {
            camera->pitch_angle = pitch_angle;
        }

        // Update direction
        camera->direction.x = sinf(camera->yaw_angle) * cosf(camera->pitch_angle);
        camera->direction.y = sinf(camera->pitch_angle);
        camera->direction.z = cosf(camera->yaw_angle) * cosf(camera->pitch_angle);
        vec3_normalize(&camera->direction);
    }
}

void set_mouse_camera_speed(brh_mouse_camera* camera, float speed)
{
    if (camera) {
        camera->speed = speed;
    }
}

float get_mouse_camera_speed(const brh_mouse_camera* camera)
{
    return camera ? camera->speed : 0.0f;
}

void set_mouse_camera_sensitivity(brh_mouse_camera* camera, float sensitivity)
{
    if (camera) {
        camera->sensitivity = sensitivity;
    }
}

float get_mouse_camera_sensitivity(const brh_mouse_camera* camera)
{
    return camera ? camera->sensitivity : 0.0f;
}