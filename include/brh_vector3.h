#pragma once


typedef struct {
	float x;
	float y;
	float z;
} brh_vector3;

float vec3_magnitude(brh_vector3 v);

brh_vector3 vec3_add(brh_vector3 a, brh_vector3 b);
brh_vector3 vec3_subtract(brh_vector3 a, brh_vector3 b);
brh_vector3 vec3_scale(brh_vector3 v, float scalar);
brh_vector3 vec3_normal(brh_vector3 v);
void vec3_normalize(brh_vector3* v);
void vec3_add_ref(brh_vector3* a, brh_vector3 b);
void vec3_subtract_ref(brh_vector3* a, brh_vector3 b);
void vec3_scale_ref(brh_vector3* v, float scalar);

float vec3_dot(brh_vector3 a, brh_vector3 b);
brh_vector3 vec3_cross(brh_vector3 a, brh_vector3 b);
float vec3_angle(brh_vector3 a, brh_vector3 b);

brh_vector3 vec3_rotate_x(brh_vector3 v, float angle);
brh_vector3 vec3_rotate_y(brh_vector3 v, float angle);
brh_vector3 vec3_rotate_z(brh_vector3 v, float angle);
brh_vector3 vec3_rotate_x_ref(brh_vector3* v, float angle);
brh_vector3 vec3_rotate_y_ref(brh_vector3* v, float angle);
brh_vector3 vec3_rotate_z_ref(brh_vector3* v, float angle);