#include <math.h>
#include "vector.h"

brh_vector3 vec3_rotate_x(brh_vector3 v, double angle)
{
	brh_vector3 rotated_vector = {
		.x = v.x,
		.y = v.y * cosf(angle) - v.z * sinf(angle),
		.z = v.y * sinf(angle) + v.z * cosf(angle),
	};
	return rotated_vector;
}

brh_vector3 vec3_rotate_y(brh_vector3 v, double angle)
{
	brh_vector3 rotated_vector = {
		.x = v.x * cosf(angle) + v.z * sinf(angle),
		.y = v.y,
		.z = -v.x * sinf(angle) + v.z * cosf(angle),
	};

	return rotated_vector;
}

brh_vector3 vec3_rotate_z(brh_vector3 v, double angle)
{
	brh_vector3 rotated_vector = {
		.x = v.x * cosf(angle) - v.y * sinf(angle),
		.y = v.x * sinf(angle) + v.y * cosf(angle),
		.z = v.z,
	};

	return rotated_vector;
}
