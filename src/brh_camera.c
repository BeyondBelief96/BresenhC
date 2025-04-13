#include <math.h>
#include <stdio.h>
#include "math_utils.h"
#include "brh_camera.h"

float frustum_fov_y = 60.0f;
float near_plane = 1.0f;
float far_plane = 100.0f;

brh_look_at_camera lookat_camera = {
	.position = { 0.0f, 0.0f, 0.0f },
	.target = { 0.0f, 0.0f, 1.0f }
};

brh_fps_camera fps_camera = {
	.position = { 0.0f, 0.0f, 0.0f },
	.direction = { 0.0f, 0.0f, 1.0f },
	.forward_velocity = { 0.0f, 0.0f, 0.0f },
	.yaw_angle = 0.0f
};

brh_mouse_camera mouse_camera = {
	.position = { 0.0f, 0.0f, 0.0f },
	.direction = { 0.0f, 0.0f, 1.0f },
	.yaw_angle = 0.0f,
	.pitch_angle = 0.0f,
	.speed = 5.0f,
	.sensitivity = 0.001f
};


brh_mat4 create_camera_matrix(brh_vector3 eyePosition, brh_vector3 target, brh_vector3 up)
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

void update_mouse_camera_view(brh_mouse_camera* camera, int mouse_x_rel, int mouse_y_rel, float delta_time)
{
    // Remove delta_time - mouse movement doesn't need to be time-scaled
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

brh_mat4 get_mouse_camera_view_matrix(brh_mouse_camera* camera)
{
	brh_vector3 target = vec3_add(camera->position, camera->direction);
	brh_vector3 up = { 0.0f, 1.0f, 0.0f };
	return create_camera_matrix(camera->position, target, up);
}
