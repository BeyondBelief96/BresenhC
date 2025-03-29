#include <math.h>
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

brh_mat4 mat4_create_rotation_x(float theta)
{
	float c = cosf(theta);
	float s = sinf(theta);
	brh_mat4 result = mat4_identity();
	result.m[1][1] = c;
	result.m[1][2] = s;
	result.m[2][1] = -s;
	result.m[2][2] = c;
	return result;
}

brh_mat4 mat4_create_rotation_y(float theta)
{
	float c = cosf(theta);
	float s = sinf(theta);
	brh_mat4 result = mat4_identity();
	result.m[0][0] = c;
	result.m[0][2] = -s;
	result.m[2][0] = s;
	result.m[2][2] = c;
	return result;
}

brh_mat4 mat4_create_rotation_z(float theta)
{
	float c = cosf(theta);
	float s = sinf(theta);
	brh_mat4 result = mat4_identity();
	result.m[0][0] =c;
	result.m[0][1] = s;
	result.m[1][0] = -s;
	result.m[1][1] = c;
	return result;
}

brh_vector4 mat4_mul_vec4(const brh_mat4* m, brh_vector4 v)
{
	brh_vector4 result = { 0 };
	result.x = m->m[0][0] * v.x + m->m[0][1] * v.y + m->m[0][2] * v.z + m->m[0][3] * v.w;
	result.y = m->m[1][0] * v.x + m->m[1][1] * v.y + m->m[1][2] * v.z + m->m[1][3] * v.w;
	result.z = m->m[2][0] * v.x + m->m[2][1] * v.y + m->m[2][2] * v.z + m->m[2][3] * v.w;
	result.w = m->m[3][0] * v.x + m->m[3][1] * v.y + m->m[3][2] * v.z + m->m[3][3] * v.w; 
	return result;
}

void mat4_mul_vec4_ref(const brh_mat4* m, brh_vector4* v)
{
	float x = m->m[0][0] * v->x + m->m[0][1] * v->y + m->m[0][2] * v->z + m->m[0][3] * v->w;
	float y = m->m[1][0] * v->x + m->m[1][1] * v->y + m->m[1][2] * v->z + m->m[1][3] * v->w;
	float z = m->m[2][0] * v->x + m->m[2][1] * v->y + m->m[2][2] * v->z + m->m[2][3] * v->w;
	float w = m->m[3][0] * v->x + m->m[3][1] * v->y + m->m[3][2] * v->z + m->m[3][3] * v->w;

	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void mat4_mul_mat4_ref(const brh_mat4* a, const brh_mat4* b, brh_mat4* result)
{
	float m00 = b->m[0][0] * a->m[0][0] + b->m[0][1] * a->m[1][0] + b->m[0][2] * a->m[2][0] + b->m[0][3] * a->m[3][0];
	float m01 = b->m[0][0] * a->m[0][1] + b->m[0][1] * a->m[1][1] + b->m[0][2] * a->m[2][1] + b->m[0][3] * a->m[3][1];
	float m02 = b->m[0][0] * a->m[0][2] + b->m[0][1] * a->m[1][2] + b->m[0][2] * a->m[2][2] + b->m[0][3] * a->m[3][2];
	float m03 = b->m[0][0] * a->m[0][3] + b->m[0][1] * a->m[1][3] + b->m[0][2] * a->m[2][3] + b->m[0][3] * a->m[3][3];

	float m10 = b->m[1][0] * a->m[0][0] + b->m[1][1] * a->m[1][0] + b->m[1][2] * a->m[2][0] + b->m[1][3] * a->m[3][0];
	float m11 = b->m[1][0] * a->m[0][1] + b->m[1][1] * a->m[1][1] + b->m[1][2] * a->m[2][1] + b->m[1][3] * a->m[3][1];
	float m12 = b->m[1][0] * a->m[0][2] + b->m[1][1] * a->m[1][2] + b->m[1][2] * a->m[2][2] + b->m[1][3] * a->m[3][2];
	float m13 = b->m[1][0] * a->m[0][3] + b->m[1][1] * a->m[1][3] + b->m[1][2] * a->m[2][3] + b->m[1][3] * a->m[3][3];

	float m20 = b->m[2][0] * a->m[0][0] + b->m[2][1] * a->m[1][0] + b->m[2][2] * a->m[2][0] + b->m[2][3] * a->m[3][0];
	float m21 = b->m[2][0] * a->m[0][1] + b->m[2][1] * a->m[1][1] + b->m[2][2] * a->m[2][1] + b->m[2][3] * a->m[3][1];
	float m22 = b->m[2][0] * a->m[0][2] + b->m[2][1] * a->m[1][2] + b->m[2][2] * a->m[2][2] + b->m[2][3] * a->m[3][2];
	float m23 = b->m[2][0] * a->m[0][3] + b->m[2][1] * a->m[1][3] + b->m[2][2] * a->m[2][3] + b->m[2][3] * a->m[3][3];

	float m30 = b->m[3][0] * a->m[0][0] + b->m[3][1] * a->m[1][0] + b->m[3][2] * a->m[2][0] + b->m[3][3] * a->m[3][0];
	float m31 = b->m[3][0] * a->m[0][1] + b->m[3][1] * a->m[1][1] + b->m[3][2] * a->m[2][1] + b->m[3][3] * a->m[3][1];
	float m32 = b->m[3][0] * a->m[0][2] + b->m[3][1] * a->m[1][2] + b->m[3][2] * a->m[2][2] + b->m[3][3] * a->m[3][2];
	float m33 = b->m[3][0] * a->m[0][3] + b->m[3][1] * a->m[1][3] + b->m[3][2] * a->m[2][3] + b->m[3][3] * a->m[3][3];

	result->m[0][0] = m00;
	result->m[0][1] = m01;
	result->m[0][2] = m02;
	result->m[0][3] = m03;

	result->m[1][0] = m10;
	result->m[1][1] = m11;
	result->m[1][2] = m12;
	result->m[1][3] = m13;

	result->m[2][0] = m20;
	result->m[2][1] = m21;
	result->m[2][2] = m22;
	result->m[2][3] = m23;

	result->m[3][0] = m30;
	result->m[3][1] = m31;
	result->m[3][2] = m32;
	result->m[3][3] = m33;
}

brh_mat4 mat4_create_world_matrix(brh_vector3 translation, brh_vector3 rotation, brh_vector3 scale)
{
	brh_mat4 scale_matrix = mat4_create_scale(scale.x, scale.y, scale.z);
	brh_mat4 rotation_matrix_x = mat4_create_rotation_x(rotation.x);
	brh_mat4 rotation_matrix_y = mat4_create_rotation_y(rotation.y);
	brh_mat4 rotation_matrix_z = mat4_create_rotation_z(rotation.z);
	brh_mat4 translation_matrix = mat4_create_translation(translation.x, translation.y, translation.z);
	brh_mat4 world_matrix = mat4_identity();

	// Apply scale first
	mat4_mul_mat4_ref(&world_matrix, &scale_matrix, &world_matrix);

	// Apply rotations in Z, Y, X order
	mat4_mul_mat4_ref(&world_matrix, &rotation_matrix_z, &world_matrix);
	mat4_mul_mat4_ref(&world_matrix, &rotation_matrix_y, &world_matrix);
	mat4_mul_mat4_ref(&world_matrix, &rotation_matrix_x, &world_matrix);

	// Apply translation last
	mat4_mul_mat4_ref(&world_matrix, &translation_matrix, &world_matrix);

	return world_matrix;
}



