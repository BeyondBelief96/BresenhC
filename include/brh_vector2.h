#pragma once


typedef struct
{
	float x;
	float y;
} brh_vector2;

float vec2_magnitude(brh_vector2 v);

brh_vector2 vec2_add(brh_vector2 a, brh_vector2 b);
brh_vector2 vec2_subtract(brh_vector2 a, brh_vector2 b);
brh_vector2 vec2_scale(brh_vector2 v, float scalar);
brh_vector2 vec2_normal(brh_vector2 v);
void vec2_normalize(brh_vector2* v);
float vec2_dot(brh_vector2 a, brh_vector2 b);
float vec2_cross(brh_vector2 a, brh_vector2 b);
float vec2_angle(brh_vector2 a, brh_vector2 b);

/* Reference modifying versions */
void vec2_add_ref(brh_vector2* a, brh_vector2 b);
void vec2_subtract_ref(brh_vector2* a, brh_vector2 b);
void vec2_scale_ref(brh_vector2* v, float scalar);

brh_vector2 vec2_rotate(brh_vector2 v, float angle);
brh_vector2 vec2_rotate_ref(brh_vector2* v, float angle);