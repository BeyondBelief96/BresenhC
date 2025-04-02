#pragma once

#include "brh_vector.h"

typedef struct brh_light {
	brh_vector3 direction;
} brh_light;

extern brh_light global_light;

uint32_t calculate_flat_shading_color(brh_vector3 normal, uint32_t originalColor);
