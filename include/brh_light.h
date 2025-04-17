#pragma once

#include "brh_vector.h"

typedef enum shading_method {
    SHADING_NONE,    // No lighting, just use original color/texture (useful debug state)
    SHADING_FLAT,    // One lighting calculation per triangle face
    SHADING_GOURAUD, // Lighting calculated per vertex, color interpolated across face
    SHADING_PHONG,   // Normal interpolated per pixel, lighting calculated per pixel
} shading_method;


typedef struct brh_light {
    brh_vector3 direction;  // Direction *towards* the light source (normalized)
    float ambient;          // Ambient light intensity (0.0 - 1.0)
    float diffuse;          // Diffuse light intensity (0.0 - 1.0)
    float specular;         // Specular light intensity (0.0 - 1.0)
    int specular_power;     // Shininess factor for specular highlights (higher means sharper)
} brh_global_light;

/*
* @brief Gets the current shading method.
*
* @return The current shading method.
*/
enum shading_method get_shading_method(void);

/**
 * @brief Sets the shading method.
 *
 * @param method The shading method to set.
 *
 * @return void
 */
void set_shading_method(enum shading_method method);

/*
* @brief Gets the global light parameters.
* 
* @return The global light parameters.
* 
* @note The global light parameters include the direction, ambient, diffuse,
*       specular light intensity, and specular power.
*/
brh_global_light get_global_light(void);

/*
* @brief Sets the global light direction.
* 
* @param direction The new light direction (normalized) towards the light source.
* 
* @return void
*/
void set_global_light_direction(brh_vector3 direction);

/*
* @brief Sets the light parameters.
* 
* @param ambient Ambient light intensity (0.0 - 1.0).
* @param diffuse Diffuse light intensity (0.0 - 1.0).
* @param specular Specular light intensity (0.0 - 1.0).
* @param specular_power Shininess factor for specular highlights (higher means sharper).
* 
* @return void
*/
void set_light_parameters(float ambient, float diffuse, float specular, int specular_power);

/**
 * @brief Calculates the final color for a face using flat shading.
 *
 * @param face_normal_world The normal vector of the triangle face in world space (normalized).
 * @param baseColor The original color of the face (e.g., from material or default).
 * @return The calculated 32-bit ARGB color.
 */
uint32_t calculate_flat_shading_color(brh_vector3 face_normal_world, uint32_t baseColor);

/**
 * @brief Calculates the final color for a vertex using Gouraud shading components.
 *
 * @param vertex_normal_world The normal vector at the vertex in world space (normalized).
 * @param vertex_pos_world The position of the vertex in world space.
 * @param camera_pos_world The position of the camera (viewer) in world space.
 * @param baseColor The original color of the vertex/face.
 * @return The calculated 32-bit ARGB color for this vertex.
 */
uint32_t calculate_vertex_shading_color(brh_vector3 vertex_normal_world, brh_vector3 vertex_pos_world, brh_vector3 camera_pos_world, uint32_t baseColor);

/**
 * @brief Calculates the final color for a pixel using Phong shading components.
 *
 * @param interpolated_normal_world The interpolated normal vector at the pixel in world space (normalized).
 * @param pixel_pos_world The position of the pixel in world space (can be estimated or interpolated).
 * @param camera_pos_world The position of the camera (viewer) in world space.
 * @param baseColor The base color for the pixel (from texture lookup or face color).
 * @return The calculated 32-bit ARGB color for this pixel.
 */
uint32_t calculate_phong_shading_color(brh_vector3 interpolated_normal_world, brh_vector3 pixel_pos_world, brh_vector3 camera_pos_world, uint32_t baseColor);


/**
 * @brief Linearly interpolates between two ARGB colors.
 *
 * @param c1 The first color.
 * @param c2 The second color.
 * @param t The interpolation factor (0.0 to 1.0).
 * @return The interpolated ARGB color.
 */
uint32_t interpolate_colors(uint32_t c1, uint32_t c2, float t);
