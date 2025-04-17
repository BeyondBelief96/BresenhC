#include <stdint.h>
#include <math.h>
#include "brh_light.h"
#include "brh_vector.h"
#include "math_utils.h"

static enum shading_method renderer_shading_method = SHADING_FLAT;

static brh_global_light global_light = {
    .direction = {.x = 0.0f, .y = 0.0f, .z = 1.0f }, // Direction the light source points.
    .ambient = 0.1f,
    .diffuse = 0.7f,
    .specular = 0.2f, // Add some default specularity
    .specular_power = 64 // Common default shininess
};


shading_method get_shading_method(void)
{
    return renderer_shading_method;
}

void set_shading_method(shading_method method)
{
	renderer_shading_method = method;
}

brh_global_light get_global_light(void)
{
    return global_light;
}

void set_global_light_direction(brh_vector3 direction)
{
    global_light.direction = vec3_unit_vector(direction);
}


void set_light_parameters(float ambient, float diffuse, float specular, int specular_power)
{
    global_light.ambient = (float)fmax(0.0f, fmin(1.0f, ambient));
    global_light.diffuse = (float)fmax(0.0f, fmin(1.0f, diffuse));
    global_light.specular = (float)fmax(0.0f, fmin(1.0f, specular));
    global_light.specular_power = (int)fmax(1, specular_power);
}

// Helper to apply intensity to a color component
static uint8_t apply_intensity(uint8_t component, float intensity) {
    float result = (float)component * intensity;
    return (uint8_t)MAX(0.0f, MIN(255.0f, result));
}

// Helper to combine color components
static uint32_t combine_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

// Helper to add two color components, clamping at 255
static uint8_t add_component_clamped(uint8_t c1, uint8_t c2) {
    int sum = (int)c1 + (int)c2;
    return (uint8_t)MIN(255, sum);
}


// --- Flat Shading Calculation ---
uint32_t calculate_flat_shading_color(brh_vector3 face_normal_world, uint32_t baseColor)
{
    brh_global_light light = get_global_light(); // Get current light settings

    // Ensure normal is normalized (should be, but safety check)
    face_normal_world = vec3_unit_vector(face_normal_world);

    // Calculate diffuse factor (Lambertian)
    // Light direction is pointing *from* the light, so we need its negative for the dot product.
    float diffuse_factor = vec3_dot(face_normal_world, vec3_scale(light.direction, -1.0));

    // Clamp negative values (light source is behind the surface)
    diffuse_factor = MAX(0.0f, diffuse_factor);

    // Calculate final intensity (Ambient + Diffuse)
    // Note: Flat shading typically doesn't include specular highlights
    float intensity = light.ambient + (light.diffuse * diffuse_factor);
    intensity = MIN(1.0f, intensity); // Clamp intensity

    // Extract base color components
    uint8_t a = (baseColor >> 24) & 0xFF;
    uint8_t r = (baseColor >> 16) & 0xFF;
    uint8_t g = (baseColor >> 8) & 0xFF;
    uint8_t b = baseColor & 0xFF;

    // Apply intensity to each component
    r = apply_intensity(r, intensity);
    g = apply_intensity(g, intensity);
    b = apply_intensity(b, intensity);

    // Combine components back into an ARGB color
    return combine_argb(a, r, g, b);
}

static void calculate_diffuse_specular(
    brh_vector3 normal_world,
    brh_vector3 point_pos_world,
    brh_vector3 camera_pos_world,
    float* out_diffuse_intensity, 
    float* out_specular_intensity)
{
	normal_world = vec3_unit_vector(normal_world);
    
    // --- Diffuse Calculation ---
	float diffuse_factor = vec3_dot(normal_world, vec3_scale(global_light.direction, -1.0));
	diffuse_factor = MAX(0.0f, diffuse_factor);
	*out_diffuse_intensity = global_light.ambient + (global_light.diffuse * diffuse_factor);

    // --- Specular Calculation ---
	*out_specular_intensity = 0.0f; // Default to zero

    if (diffuse_factor > EPSILON && global_light.specular > EPSILON && global_light.specular_power > 0)
    {
        // Reflection Vector R = 2 * (N.L) * N - L
        brh_vector3 L = global_light.direction;
		brh_vector3 R = vec3_subtract(vec3_scale(normal_world, 2.0f * diffuse_factor), L);
		R = vec3_unit_vector(R);

		// View Vector V = camera_pos - point_pos
		brh_vector3 V = vec3_subtract(camera_pos_world, point_pos_world);
		V = vec3_unit_vector(V);

        // Specular intensity = (R . V)^shininess
        float R_dot_V = vec3_dot(R, V);
        if (R_dot_V > EPSILON) { // Check if reflection is towards the viewer
            float specular_factor = powf(R_dot_V, (float)global_light.specular_power);
            *out_specular_intensity = global_light.specular * specular_factor;
        }
    }
}


// --- Vertex Shading Calculation (Gouraud) ---
uint32_t calculate_vertex_shading_color(brh_vector3 vertex_normal_world, brh_vector3 vertex_pos_world, brh_vector3 camera_pos_world, uint32_t baseColor)
{
    brh_global_light light = get_global_light();
    float diffuse_intensity, specular_intensity;

    calculate_diffuse_specular(
        vertex_normal_world,
        vertex_pos_world,
        camera_pos_world,
        &diffuse_intensity,
        &specular_intensity
    );

    // Extract base color components
    uint8_t a_base = (baseColor >> 24) & 0xFF;
    uint8_t r_base = (baseColor >> 16) & 0xFF;
    uint8_t g_base = (baseColor >> 8) & 0xFF;
    uint8_t b_base = baseColor & 0xFF;

    // Calculate ambient and diffuse color components
    float ambient_diffuse_intensity = light.ambient + diffuse_intensity;
    uint8_t r_ad = apply_intensity(r_base, ambient_diffuse_intensity);
    uint8_t g_ad = apply_intensity(g_base, ambient_diffuse_intensity);
    uint8_t b_ad = apply_intensity(b_base, ambient_diffuse_intensity);

    // Calculate specular color component (usually white or light color)
    // Let's use white specular for simplicity for now
    uint8_t r_spec = apply_intensity(255, specular_intensity);
    uint8_t g_spec = apply_intensity(255, specular_intensity);
    uint8_t b_spec = apply_intensity(255, specular_intensity);

    // Add components together, clamping
    uint8_t r_final = add_component_clamped(r_ad, r_spec);
    uint8_t g_final = add_component_clamped(g_ad, g_spec);
    uint8_t b_final = add_component_clamped(b_ad, b_spec);

    return combine_argb(a_base, r_final, g_final, b_final);
}

// --- Pixel Shading Calculation (Phong) ---
uint32_t calculate_phong_shading_color(brh_vector3 interpolated_normal_world, brh_vector3 pixel_pos_world, brh_vector3 camera_pos_world, uint32_t baseColor)
{
    // Phong calculation is identical to Gouraud at the point level,
    // the difference is *when* it's calculated (per-pixel vs per-vertex)
    // So we can reuse the vertex shading logic here.
    return calculate_vertex_shading_color(
        interpolated_normal_world,
        pixel_pos_world,
        camera_pos_world,
        baseColor
    );
}

// --- Color Interpolation ---
uint32_t interpolate_colors(uint32_t c1, uint32_t c2, float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    uint8_t a1 = (c1 >> 24) & 0xFF; uint8_t r1 = (c1 >> 16) & 0xFF; uint8_t g1 = (c1 >> 8) & 0xFF; uint8_t b1 = c1 & 0xFF;
    uint8_t a2 = (c2 >> 24) & 0xFF; uint8_t r2 = (c2 >> 16) & 0xFF; uint8_t g2 = (c2 >> 8) & 0xFF; uint8_t b2 = c2 & 0xFF;

    uint8_t a = (uint8_t)interpolate_float((float)a1, (float)a2, t);
    uint8_t r = (uint8_t)interpolate_float((float)r1, (float)r2, t);
    uint8_t g = (uint8_t)interpolate_float((float)g1, (float)g2, t);
    uint8_t b = (uint8_t)interpolate_float((float)b1, (float)b2, t);

    return combine_argb(a, r, g, b);
}
