#pragma once

#include "brh_vector.h"

typedef struct {
	float m[4][4];
} brh_mat4;

brh_mat4 mat4_identity(void);
brh_mat4 mat4_create_scale(float sx, float sy, float sz);
brh_mat4 mat4_create_translation(float tx, float ty, float tz);
brh_vector4 mat4_mul_vec4(brh_mat4 m, brh_vector4 v);
void mat4_mul_vec4_ref(brh_mat4 m, brh_vector4* v);