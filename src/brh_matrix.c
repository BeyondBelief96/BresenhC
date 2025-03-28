#include "brh_matrix.h"

brh_mat4 mat4_identity(void)
{
	brh_mat4 result = { 0 };
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

brh_mat4 mat4_create_scale(float sx, float sy, float sz)
{
	brh_mat4 result = mat4_identity();
	result.m[0][0] = sx;
	result.m[1][1] = sy;
	result.m[2][2] = sz;
	return result;
}

brh_mat4 mat4_create_translation(float tx, float ty, float tz)
{
	brh_mat4 result = mat4_identity();
	result.m[0][3] = tx;
	result.m[1][3] = ty;
	result.m[2][3] = tz;
	return result;
}

brh_vector4 mat4_mul_vec4(brh_mat4 m, brh_vector4 v)
{
	brh_vector4 result = { 0 };
	result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
	result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
	result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
	result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w; 
	return result;
}

void mat4_mul_vec4_ref(brh_mat4 m, brh_vector4* v)
{
	v->x = m.m[0][0] * v->x + m.m[0][1] * v->y + m.m[0][2] * v->z + m.m[0][3] * v->w;
	v->y = m.m[1][0] * v->x + m.m[1][1] * v->y + m.m[1][2] * v->z + m.m[1][3] * v->w;
	v->z = m.m[2][0] * v->x + m.m[2][1] * v->y + m.m[2][2] * v->z + m.m[2][3] * v->w;
	v->w = m.m[3][0] * v->x + m.m[3][1] * v->y + m.m[3][2] * v->z + m.m[3][3] * v->w;
}