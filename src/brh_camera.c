#include "brh_camera.h"

brh_look_at_camera lookat_camera = {
	.position = { 0.0f, 0.0f, 0.0f },
	.target = { 0.0f, 0.0f, 1.0f }
};


brh_mat4 create_look_at_camera_matrix(brh_vector3 eyePosition, brh_vector3 target, brh_vector3 up)
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
