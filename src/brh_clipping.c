#include <math.h>
#include "brh_clipping.h"

brh_plane clipping_planes[6];

void initialize_frustum_clipping_planes(float fov, float near_plane, float far_plane)
{
	brh_vector3 origin = { .x = 0.0f, .y = 0.0f, .z = 0.0f };
	brh_vector3 nearPoint = { .x = 0.0f, .y = 0.0f, .z = near_plane };
	brh_vector3 farPoint = { .x = 0.0f, .y = 0.0f, .z = far_plane };

	float cos_fov = cosf(fov / 2.0f);
	float sin_fov = sinf(fov / 2.0f);
	clipping_planes[LEFT_PLANE].point = origin;
	clipping_planes[LEFT_PLANE].normal.x = cos_fov;
	clipping_planes[LEFT_PLANE].normal.y = 0.0f;
	clipping_planes[LEFT_PLANE].normal.z = sin_fov;

	clipping_planes[RIGHT_PLANE].point = origin;
	clipping_planes[RIGHT_PLANE].normal.x = -cos_fov;
	clipping_planes[RIGHT_PLANE].normal.y = 0.0f;
	clipping_planes[RIGHT_PLANE].normal.z = sin_fov;

	clipping_planes[TOP_PLANE].point = origin;
	clipping_planes[TOP_PLANE].normal.x = 0.0f;
	clipping_planes[TOP_PLANE].normal.y = -cos_fov;
	clipping_planes[TOP_PLANE].normal.z = sin_fov;

	clipping_planes[BOTTOM_PLANE].point = origin;
	clipping_planes[BOTTOM_PLANE].normal.x = 0.0f;
	clipping_planes[BOTTOM_PLANE].normal.y = cos_fov;
	clipping_planes[BOTTOM_PLANE].normal.z = sin_fov;

	clipping_planes[NEAR_PLANE].point = nearPoint;
	clipping_planes[NEAR_PLANE].normal.x = 0.0f;
	clipping_planes[NEAR_PLANE].normal.y = 0.0f;
	clipping_planes[NEAR_PLANE].normal.z = 1.0f;

	clipping_planes[FAR_PLANE].point = farPoint;
	clipping_planes[FAR_PLANE].normal.x = 0.0f;
	clipping_planes[FAR_PLANE].normal.y = 0.0f;
	clipping_planes[FAR_PLANE].normal.z = -1.0f;
}

brh_vector3 find_line_plane_intersection(brh_vector3 line_start, brh_vector3 line_end, brh_plane plane)
{
	brh_vector3 q1_p = vec3_subtract(line_start, plane.point);
	brh_vector3 q2_p = vec3_subtract(line_end, plane.point);
	float d1 = vec3_dot(q1_p, plane.normal);
	float d2 = vec3_dot(q2_p, plane.normal);
	float t = d1 / (d1 - d2);

	brh_vector3 intersection = {
		.x = line_start.x + t * (line_end.x - line_start.x),
		.y = line_start.y + t * (line_end.y - line_start.y),
		.z = line_start.z + t * (line_end.z - line_start.z)
	};

	return intersection;
}
