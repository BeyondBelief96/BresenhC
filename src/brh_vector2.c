#include <math.h>
#include "brh_vector2.h"

float vec2_magnitude(brh_vector2 v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
};

brh_vector2 vec2_add(brh_vector2 a, brh_vector2 b)
{
	brh_vector2 sum = {
		.x = a.x + b.x,
		.y = a.y + b.y,
	};
	return sum;
}

brh_vector2 vec2_subtract(brh_vector2 a, brh_vector2 b)
{
	brh_vector2 difference = {
		.x = a.x - b.x,
		.y = a.y - b.y,
	};
	return difference;
}

brh_vector2 vec2_scale(brh_vector2 v, float scalar)
{
	brh_vector2 scaled_vector = {
		.x = v.x * scalar,
		.y = v.y * scalar,
	};
	return scaled_vector;
}

void vec2_add_ref(brh_vector2* a, brh_vector2 b)
{
	a->x += b.x;
	a->y += b.y;
}

void vec2_subtract_ref(brh_vector2* a, brh_vector2 b)
{
	a->x -= b.x;
	a->y -= b.y;
}

void vec2_scale_ref(brh_vector2* v, float scalar)
{
	v->x *= scalar;
	v->y *= scalar;
}

float vec2_dot(brh_vector2 a, brh_vector2 b)
{
	return a.x * b.x + a.y * b.y;
}


float vec2_cross(brh_vector2 a, brh_vector2 b)
{
	return a.x * b.y - a.y * b.x;
}

float vec2_angle(brh_vector2 a, brh_vector2 b)
{
	float dot = vec2_dot(a, b);
	float a_magnitude = vec2_magnitude(a);
	float b_magnitude = vec2_magnitude(b);
	return acosf(dot / (a_magnitude * b_magnitude));
}

brh_vector2 vec2_normal(brh_vector2 v)
{
	float magnitude = vec2_magnitude(v);
	brh_vector2 normalized_vector = {
		.x = v.x / magnitude,
		.y = v.y / magnitude,
	};
	return normalized_vector;
}

void vec2_normalize(brh_vector2* v)
{
	float magnitude = vec2_magnitude(*v);
	v->x /= magnitude;
	v->y /= magnitude;
}

brh_vector2 vec2_rotate(brh_vector2 v, float angle)
{
	brh_vector2 rotated_vector = {
		.x = v.x * cosf(angle) - v.y * sinf(angle),
		.y = v.x * sinf(angle) + v.y * cosf(angle),
	};
	return rotated_vector;
}

brh_vector2 vec2_rotate_ref(brh_vector2* v, float angle)
{
	float x = v->x;
	v->x = x * cosf(angle) - v->y * sinf(angle);
	v->y = x * sinf(angle) + v->y * cosf(angle);
	return *v;
}