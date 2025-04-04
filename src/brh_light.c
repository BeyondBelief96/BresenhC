#include <stdint.h>
#include "brh_light.h"
#include "brh_vector.h"

brh_light global_light = {
	.direction = {.x = 0.0f, .y = 0.0f, .z = 1.0f }
};

uint32_t calculate_flat_shading_color(brh_vector3 normal, uint32_t originalColor)
{
    // Use the negative of the light direction vector
    // This way, dot product is positive when face points toward light
    float intensity = vec3_dot(normal, vec3_scale((global_light.direction), -1));

    // Clamp intensity to ensure it's between 0 and 1
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;

    // Extract color components
    uint32_t alpha = (originalColor & 0xFF000000);
    uint32_t red = (uint32_t)((originalColor & 0x00FF0000) * intensity) & 0x00FF0000;
    uint32_t green = (uint32_t)((originalColor & 0x0000FF00) * intensity) & 0x0000FF00;
    uint32_t blue = (uint32_t)((originalColor & 0x000000FF) * intensity) & 0x000000FF;

    // Combine components back into a rgba color
    uint32_t color = alpha | red | green | blue;

    return color;
}