#pragma once

#include "brh_vector.h"

typedef struct brh_light {
	brh_vector3 direction;
} brh_global_light;

/*
* @brief Gets the global light object.
* 
* This function retrieves the global light object used for shading.
* 
* @return The global light object.
*/
brh_global_light get_global_light(void);

/*
* @brief Sets the global light direction.
* 
* This function sets the direction of the global light source used for shading.
* 
* @param direction The direction of the light source as a brh_vector3.
* 
* @return void
*/
void set_global_light_direction(brh_vector3 direction);

/*
* @brief Calculates the flat shading color based on the normal and original color.
* 
* This function computes the flat shading color for a triangle based on its normal
* and the original color. The light direction is assumed to be in the negative Z direction.
* 
* @param normal The normal vector of the triangle as a brh_vector3.
* @param originalColor The original color of the triangle in ARGB format.
* 
* @return The calculated color in ARGB format.
*/
uint32_t calculate_flat_shading_color(brh_vector3 normal, uint32_t originalColor);
