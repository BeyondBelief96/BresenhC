#pragma once

typedef struct
{
	double x;
	double y;
} brh_vector2;

typedef struct {
	double x;
	double y;
	double z;
} brh_vector3;

brh_vector3 vec3_rotate_x(brh_vector3 v, double angle);
brh_vector3 vec3_rotate_y(brh_vector3 v, double angle);
brh_vector3 vec3_rotate_z(brh_vector3 v, double angle);