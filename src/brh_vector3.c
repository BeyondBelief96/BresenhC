#include <math.h>
#include "brh_vector3.h"

float vec3_magnitude(brh_vector3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

brh_vector3 vec3_add(brh_vector3 a, brh_vector3 b)
{
	brh_vector3 sum = {
		.x = a.x + b.x,
		.y = a.y + b.y,
		.z = a.z + b.z,
	};

	return sum;
}

brh_vector3 vec3_subtract(brh_vector3 a, brh_vector3 b)
{
	brh_vector3 difference = {
		.x = a.x - b.x,
		.y = a.y - b.y,
		.z = a.z - b.z,
	};
	return difference;
}

brh_vector3 vec3_scale(brh_vector3 v, float scalar)
{
	brh_vector3 scaled_vector = {
		.x = v.x * scalar,
		.y = v.y * scalar,
		.z = v.z * scalar,
	};
	return scaled_vector;
}

void vec3_add_ref(brh_vector3* a, brh_vector3 b)
{
	a->x += b.x;
	a->y += b.y;
	a->z += b.z;
}

void vec3_subtract_ref(brh_vector3* a, brh_vector3 b)
{
	a->x -= b.x;
	a->y -= b.y;
	a->z -= b.z;
}

void vec3_scale_ref(brh_vector3* v, float scalar)
{
	v->x *= scalar;
	v->y *= scalar;
	v->z *= scalar;
}

float vec3_dot(brh_vector3 a, brh_vector3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

brh_vector3 vec3_cross(brh_vector3 a, brh_vector3 b)
{
	brh_vector3 cross_product = {
		.x = a.y * b.z - a.z * b.y,
		.y = a.z * b.x - a.x * b.z,
		.z = a.x * b.y - a.y * b.x,
	};
	return cross_product;
}

brh_vector3 vec3_normal(brh_vector3 v)
{
	float magnitude = vec3_magnitude(v);
	brh_vector3 normalized_vector = {
		.x = v.x / magnitude,
		.y = v.y / magnitude,
		.z = v.z / magnitude,
	};
	return normalized_vector;
}

void vec3_normalize(brh_vector3* v)
{
	float magnitude = vec3_magnitude(*v);
	v->x /= magnitude;
	v->y /= magnitude;
	v->z /= magnitude;
}

float vec3_angle(brh_vector3 a, brh_vector3 b)
{
	float dot = vec3_dot(a, b);
	float a_magnitude = vec3_magnitude(a);
	float b_magnitude = vec3_magnitude(b);
	return acosf(dot / (a_magnitude * b_magnitude));
}

brh_vector3 vec3_rotate_x(brh_vector3 v, float angle)
{
	brh_vector3 rotated_vector = {
		.x = v.x,
		.y = v.y * cosf(angle) - v.z * sinf(angle),
		.z = v.y * sinf(angle) + v.z * cosf(angle),
	};
	return rotated_vector;
}

brh_vector3 vec3_rotate_y(brh_vector3 v, float angle)
{
	brh_vector3 rotated_vector = {
		.x = v.x * cosf(angle) + v.z * sinf(angle),
		.y = v.y,
		.z = -v.x * sinf(angle) + v.z * cosf(angle),
	};

	return rotated_vector;
}

brh_vector3 vec3_rotate_z(brh_vector3 v, float angle)
{
	brh_vector3 rotated_vector = {
		.x = v.x * cosf(angle) - v.y * sinf(angle),
		.y = v.x * sinf(angle) + v.y * cosf(angle),
		.z = v.z,
	};

	return rotated_vector;
}

brh_vector3 vec3_rotate_x_ref(brh_vector3* v, float angle)
{
	float temp_y = v->y;
	v->y = v->y * cosf(angle) - v->z * sinf(angle);
	v->z = temp_y * sinf(angle) + v->z * cosf(angle);
	return *v;
}

brh_vector3 vec3_rotate_y_ref(brh_vector3* v, float angle)
{
	float temp_x = v->x;
	v->x = v->x * cosf(angle) + v->z * sinf(angle);
	v->z = -temp_x * sinf(angle) + v->z * cosf(angle);
	return *v;
}

brh_vector3 vec3_rotate_z_ref(brh_vector3* v, float angle)
{
	float temp_x = v->x;
	v->x = v->x * cosf(angle) - v->y * sinf(angle);
	v->y = temp_x * sinf(angle) + v->y * cosf(angle);
	return *v;
}
