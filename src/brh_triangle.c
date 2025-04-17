#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "brh_triangle.h"
#include "brh_vector.h"
#include "math_utils.h"
#include "brh_display.h"
#include "brh_light.h" 

// --- Forward Declarations for new/updated rasterizers ---
static void fill_flat_bottom_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color); // Pass base color for solid fill

static void fill_flat_top_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color); // Pass base color for solid fill

static void texture_flat_bottom_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int texture_width, int texture_height);

static void texture_flat_top_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int texture_width, int texture_height);


// --- Updated swap_perspective_attribs ---
void swap_perspective_attribs(brh_perspective_attribs* a, brh_perspective_attribs* b) {
    brh_perspective_attribs temp = *a;
    *a = *b;
    *b = temp;
}

// --- Helper function to get pixel color based on shading mode ---
static uint32_t get_pixel_color(
    shading_method shading,
    brh_perspective_attribs current_attrib,
    uint32_t base_or_flat_color, // For flat shading or as base for fill modes
    const uint32_t* texture, int tex_width, int tex_height) // Null if not textured
{
    // --- Perspective Correction ---
    if (fabsf(current_attrib.inv_w) < EPSILON) {
        return 0; // Skip transparent/invalid pixel
    }
    const float current_w = 1.0f / current_attrib.inv_w;

    // --- Base Color Determination ---
    uint32_t base_color = base_or_flat_color; // Default to passed color

    // Sample texture if applicable (and not flat shading, which uses pre-calculated color)
    if (texture != NULL && shading != SHADING_FLAT) {
        const float current_u = current_attrib.u_over_w * current_w;
        const float current_v = current_attrib.v_over_w * current_w;

        // Texture Sampling (Wrap mode)
        int tex_x = (int)floorf(current_u * (float)tex_width); // Cast width to float
        // FIX 1: Reintroduce V coordinate flip if your texture coords expect it
        int tex_y = (int)floorf((1.0f - current_v) * (float)tex_height); // Cast height to float

        // Modulo wrapping
        tex_x = ((tex_x % tex_width) + tex_width) % tex_width;
        tex_y = ((tex_y % tex_height) + tex_height) % tex_height;

        // Ensure coordinates are within bounds after modulo (should be, but safety)
        if (tex_x >= 0 && tex_x < tex_width && tex_y >= 0 && tex_y < tex_height) {
            base_color = texture[tex_y * tex_width + tex_x];
        }
        else {
            // This case should ideally not happen with proper modulo, but handle defensively
            base_color = 0xFFFF00FF; // Error color (magenta)
        }
    }

    // --- Shading Calculation ---
    switch (shading) {
    case SHADING_NONE:
        // No lighting calculation, just return the base color (from texture or flat/face color)
        return base_color;

    case SHADING_FLAT:
        // Flat shading color was pre-calculated and passed in base_or_flat_color
        return base_or_flat_color;

    case SHADING_GOURAUD: {
        // Get the base color components (could be from texture or solid fill)
        uint8_t a_base = (base_color >> 24) & 0xFF;
        uint8_t r_base = (base_color >> 16) & 0xFF;
        uint8_t g_base = (base_color >> 8) & 0xFF;
        uint8_t b_base = base_color & 0xFF;

        // Get the interpolated Gouraud lighting color components
        float r_light = current_attrib.r_over_w * current_w;
        float g_light = current_attrib.g_over_w * current_w;
        float b_light = current_attrib.b_over_w * current_w;

        // FIX 2: Modulate base color by the light color.
        // Treat light color components as intensity factors (0-255 -> 0.0-1.0)
        float r_intensity = MAX(0.0f, MIN(1.0f, r_light / 255.0f));
        float g_intensity = MAX(0.0f, MIN(1.0f, g_light / 255.0f));
        float b_intensity = MAX(0.0f, MIN(1.0f, b_light / 255.0f));

        // Apply the intensity to the base color components
        uint8_t R = (uint8_t)((float)r_base * r_intensity);
        uint8_t G = (uint8_t)((float)g_base * g_intensity);
        uint8_t B = (uint8_t)((float)b_base * b_intensity);

        // Combine back into ARGB
        return ((uint32_t)a_base << 24) | ((uint32_t)R << 16) | ((uint32_t)G << 8) | B;
    }

    case SHADING_PHONG: {
        // Interpolate normal
        float nx = current_attrib.nx_over_w * current_w;
        float ny = current_attrib.ny_over_w * current_w;
        float nz = current_attrib.nz_over_w * current_w;
        brh_vector3 interpolated_normal = vec3_unit_vector((brh_vector3) { nx, ny, nz });

        // --- Phong calculation needs refinement ---
        // TODO: Interpolate world position and pass camera position for accurate Phong.
        brh_vector3 dummy_pixel_pos = { 0,0,0 };
        brh_vector3 dummy_camera_pos = { 0,0,-10 };

        // Calculate the Phong lit color using the base color (which might be from texture)
        return calculate_phong_shading_color(
            interpolated_normal,
            dummy_pixel_pos,
            dummy_camera_pos,
            base_color // Pass the potentially textured base color here
        );
    }
    default:
        return base_color; // Fallback
    }
}


// --- Updated Perspective Rasterizer (Example: Flat Bottom Textured) ---
// Note: The fill versions will be almost identical, just passing NULL for texture.
static void texture_flat_bottom_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int texture_width, int texture_height)
{
    const int window_width = get_window_width();
    const int window_height = get_window_height();
    shading_method current_shading = get_shading_method(); // Get current shading mode

    // Calculate inverse screen slopes for X coordinates
    float inv_slope_x_left = calculate_inverse_slope(x0, y0, x1, y1);
    float inv_slope_x_right = calculate_inverse_slope(x0, y0, x2, y2);

    // Calculate vertical height for interpolation factor
    const float y_height = (float)(y1 - y0);
    if (fabsf(y_height) < EPSILON) return;
    const float inv_y_height = 1.0f / y_height;

    // Scan vertically from top to bottom
    for (int y = y0; y <= y1; y++)
    {
        // Calculate vertical interpolation factor 't'
        const float t = (float)(y - y0) * inv_y_height;

        // Interpolate *all* perspective attributes along the edges
        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        attrib_left.u_over_w = interpolate_float(pa0.u_over_w, pa1.u_over_w, t);
        attrib_left.v_over_w = interpolate_float(pa0.v_over_w, pa1.v_over_w, t);
        attrib_left.r_over_w = interpolate_float(pa0.r_over_w, pa1.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa0.g_over_w, pa1.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa0.b_over_w, pa1.b_over_w, t);
        attrib_left.nx_over_w = interpolate_float(pa0.nx_over_w, pa1.nx_over_w, t);
        attrib_left.ny_over_w = interpolate_float(pa0.ny_over_w, pa1.ny_over_w, t);
        attrib_left.nz_over_w = interpolate_float(pa0.nz_over_w, pa1.nz_over_w, t);

        attrib_right.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        attrib_right.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, t);
        attrib_right.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, t);
        attrib_right.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, t);
        attrib_right.nx_over_w = interpolate_float(pa0.nx_over_w, pa2.nx_over_w, t);
        attrib_right.ny_over_w = interpolate_float(pa0.ny_over_w, pa2.ny_over_w, t);
        attrib_right.nz_over_w = interpolate_float(pa0.nz_over_w, pa2.nz_over_w, t);

        // Get integer screen X coordinates for the scanline span edges
        // Use interpolated X values directly instead of incremental updates for precision
        float current_edge_x_left_f = interpolate_float((float)x0, (float)x1, t);
        float current_edge_x_right_f = interpolate_float((float)x0, (float)x2, t);

        int x_scan_start = (int)roundf(current_edge_x_left_f);
        int x_scan_end = (int)roundf(current_edge_x_right_f);

        // Ensure left-to-right and swap interpolated attributes if needed
        if (x_scan_start > x_scan_end) {
            swap_int(&x_scan_start, &x_scan_end);
            swap_perspective_attribs(&attrib_left, &attrib_right);
        }

        // Calculate horizontal step for all attributes
        const float x_scan_width = (float)(x_scan_end - x_scan_start);
        float inv_w_step = 0, u_step = 0, v_step = 0, r_step = 0, g_step = 0, b_step = 0;
        float nx_step = 0, ny_step = 0, nz_step = 0;

        if (fabsf(x_scan_width) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            u_step = (attrib_right.u_over_w - attrib_left.u_over_w) * inv_x_scan_width;
            v_step = (attrib_right.v_over_w - attrib_left.v_over_w) * inv_x_scan_width;
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            nx_step = (attrib_right.nx_over_w - attrib_left.nx_over_w) * inv_x_scan_width;
            ny_step = (attrib_right.ny_over_w - attrib_left.ny_over_w) * inv_x_scan_width;
            nz_step = (attrib_right.nz_over_w - attrib_left.nz_over_w) * inv_x_scan_width;
        }

        // Initialize attribute interpolator for the current pixel
        brh_perspective_attribs current_attrib = attrib_left;

        // Draw the horizontal span with Z-test and shading
        for (int x = x_scan_start; x <= x_scan_end; x++)
        {
            // Bounds check
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                const float current_depth = current_attrib.inv_w; // Use 1/w for depth

                // Z-Buffer Test: Check if pixel is valid and closer than existing depth
                // Larger 1/w means closer.
                if (current_depth > get_z_buffer_at(x, y)) // Removed fabsf check - allow slightly negative?
                {
                    // Get final pixel color based on shading mode and attributes
                    uint32_t pixel_color = get_pixel_color(current_shading, current_attrib, 0 /*Not used for textured*/, texture, texture_width, texture_height);

                    // Draw pixel if color is not fully transparent (optional)
                    if ((pixel_color >> 24) > 0) {
                        draw_pixel(x, y, pixel_color);
                        set_z_buffer_at(x, y, current_depth); // Update depth buffer
                    }
                }
            }
            // Increment horizontal interpolators for the next pixel
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_step;
            current_attrib.v_over_w += v_step;
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_attrib.nx_over_w += nx_step;
            current_attrib.ny_over_w += ny_step;
            current_attrib.nz_over_w += nz_step;
        }
    }
}


// --- Updated Perspective Rasterizer (Example: Flat Top Textured) ---
// Similar updates needed for texture_flat_top_triangle_perspective
static void texture_flat_top_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int texture_width, int texture_height)
{
    const int window_width = get_window_width();
    const int window_height = get_window_height();
    shading_method current_shading = get_shading_method();

    // Slopes from bottom vertex (x2, y2) up to (x0, y0) and (x1, y1)
    float inv_slope_x_left = calculate_inverse_slope(x2, y2, x0, y0);
    float inv_slope_x_right = calculate_inverse_slope(x2, y2, x1, y1);

    const float y_height = (float)(y2 - y0); // y2 is bottom, y0 is top
    if (fabsf(y_height) < EPSILON) return;
    const float inv_y_height = 1.0f / y_height;

    // Scan vertically from bottom up to top
    for (int y = y2; y >= y0; y--)
    {
        // Interpolation factor 't' (0 at y2, 1 at y0)
        const float t = (float)(y2 - y) * inv_y_height;

        // Interpolate attributes along edges (from pa2 to pa0 and pa1)
        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        attrib_left.u_over_w = interpolate_float(pa2.u_over_w, pa0.u_over_w, t);
        attrib_left.v_over_w = interpolate_float(pa2.v_over_w, pa0.v_over_w, t);
        attrib_left.r_over_w = interpolate_float(pa2.r_over_w, pa0.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa2.g_over_w, pa0.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa2.b_over_w, pa0.b_over_w, t);
        attrib_left.nx_over_w = interpolate_float(pa2.nx_over_w, pa0.nx_over_w, t);
        attrib_left.ny_over_w = interpolate_float(pa2.ny_over_w, pa0.ny_over_w, t);
        attrib_left.nz_over_w = interpolate_float(pa2.nz_over_w, pa0.nz_over_w, t);

        attrib_right.inv_w = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        attrib_right.u_over_w = interpolate_float(pa2.u_over_w, pa1.u_over_w, t);
        attrib_right.v_over_w = interpolate_float(pa2.v_over_w, pa1.v_over_w, t);
        attrib_right.r_over_w = interpolate_float(pa2.r_over_w, pa1.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa2.g_over_w, pa1.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa2.b_over_w, pa1.b_over_w, t);
        attrib_right.nx_over_w = interpolate_float(pa2.nx_over_w, pa1.nx_over_w, t);
        attrib_right.ny_over_w = interpolate_float(pa2.ny_over_w, pa1.ny_over_w, t);
        attrib_right.nz_over_w = interpolate_float(pa2.nz_over_w, pa1.nz_over_w, t);

        // Get integer screen X coordinates for the scanline span edges
        // Interpolate from bottom vertex position
        float current_edge_x_left_f = interpolate_float((float)x2, (float)x0, t);
        float current_edge_x_right_f = interpolate_float((float)x2, (float)x1, t);

        int x_scan_start = (int)roundf(current_edge_x_left_f);
        int x_scan_end = (int)roundf(current_edge_x_right_f);

        if (x_scan_start > x_scan_end) {
            swap_int(&x_scan_start, &x_scan_end);
            swap_perspective_attribs(&attrib_left, &attrib_right);
        }

        // Calculate horizontal steps
        const float x_scan_width = (float)(x_scan_end - x_scan_start);
        float inv_w_step = 0, u_step = 0, v_step = 0, r_step = 0, g_step = 0, b_step = 0;
        float nx_step = 0, ny_step = 0, nz_step = 0;

        if (fabsf(x_scan_width) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            u_step = (attrib_right.u_over_w - attrib_left.u_over_w) * inv_x_scan_width;
            v_step = (attrib_right.v_over_w - attrib_left.v_over_w) * inv_x_scan_width;
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            nx_step = (attrib_right.nx_over_w - attrib_left.nx_over_w) * inv_x_scan_width;
            ny_step = (attrib_right.ny_over_w - attrib_left.ny_over_w) * inv_x_scan_width;
            nz_step = (attrib_right.nz_over_w - attrib_left.nz_over_w) * inv_x_scan_width;
        }

        brh_perspective_attribs current_attrib = attrib_left;

        // Draw the horizontal span
        for (int x = x_scan_start; x <= x_scan_end; x++)
        {
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                const float current_depth = current_attrib.inv_w;
                if (current_depth > get_z_buffer_at(x, y))
                {
                    uint32_t pixel_color = get_pixel_color(current_shading, current_attrib, 0, texture, texture_width, texture_height);
                    if ((pixel_color >> 24) > 0) {
                        draw_pixel(x, y, pixel_color);
                        set_z_buffer_at(x, y, current_depth);
                    }
                }
            }
            // Increment horizontal interpolators
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_step;
            current_attrib.v_over_w += v_step;
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_attrib.nx_over_w += nx_step;
            current_attrib.ny_over_w += ny_step;
            current_attrib.nz_over_w += nz_step;
        }
    }
}


// --- New Perspective Rasterizer (Example: Flat Bottom Filled) ---
static void fill_flat_bottom_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color) // Pass base color for solid fill
{
    // This function is almost identical to texture_flat_bottom_triangle_perspective,
    // but calls get_pixel_color with texture = NULL and passes base_color.
    const int window_width = get_window_width();
    const int window_height = get_window_height();
    shading_method current_shading = get_shading_method();

    float inv_slope_x_left = calculate_inverse_slope(x0, y0, x1, y1);
    float inv_slope_x_right = calculate_inverse_slope(x0, y0, x2, y2);

    const float y_height = (float)(y1 - y0);
    if (fabsf(y_height) < EPSILON) return;
    const float inv_y_height = 1.0f / y_height;

    for (int y = y0; y <= y1; y++)
    {
        const float t = (float)(y - y0) * inv_y_height;
        brh_perspective_attribs attrib_left, attrib_right;
        // Interpolate ALL attributes (inv_w, r/g/b/w, nx/ny/nz/w - u/v/w are unused but harmless)
        attrib_left.inv_w = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        // ... interpolate r/g/b/w and nx/ny/nz/w ...
        attrib_left.r_over_w = interpolate_float(pa0.r_over_w, pa1.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa0.g_over_w, pa1.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa0.b_over_w, pa1.b_over_w, t);
        attrib_left.nx_over_w = interpolate_float(pa0.nx_over_w, pa1.nx_over_w, t);
        attrib_left.ny_over_w = interpolate_float(pa0.ny_over_w, pa1.ny_over_w, t);
        attrib_left.nz_over_w = interpolate_float(pa0.nz_over_w, pa1.nz_over_w, t);


        attrib_right.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        // ... interpolate r/g/b/w and nx/ny/nz/w ...
        attrib_right.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, t);
        attrib_right.nx_over_w = interpolate_float(pa0.nx_over_w, pa2.nx_over_w, t);
        attrib_right.ny_over_w = interpolate_float(pa0.ny_over_w, pa2.ny_over_w, t);
        attrib_right.nz_over_w = interpolate_float(pa0.nz_over_w, pa2.nz_over_w, t);


        float current_edge_x_left_f = interpolate_float((float)x0, (float)x1, t);
        float current_edge_x_right_f = interpolate_float((float)x0, (float)x2, t);

        int x_scan_start = (int)roundf(current_edge_x_left_f);
        int x_scan_end = (int)roundf(current_edge_x_right_f);

        if (x_scan_start > x_scan_end) {
            swap_int(&x_scan_start, &x_scan_end);
            swap_perspective_attribs(&attrib_left, &attrib_right);
        }

        // Calculate horizontal steps for inv_w, r/g/b/w, nx/ny/nz/w
        const float x_scan_width = (float)(x_scan_end - x_scan_start);
        float inv_w_step = 0, r_step = 0, g_step = 0, b_step = 0;
        float nx_step = 0, ny_step = 0, nz_step = 0;

        if (fabsf(x_scan_width) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            // ... calculate steps for r/g/b/w and nx/ny/nz/w ...
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            nx_step = (attrib_right.nx_over_w - attrib_left.nx_over_w) * inv_x_scan_width;
            ny_step = (attrib_right.ny_over_w - attrib_left.ny_over_w) * inv_x_scan_width;
            nz_step = (attrib_right.nz_over_w - attrib_left.nz_over_w) * inv_x_scan_width;
        }


        brh_perspective_attribs current_attrib = attrib_left;

        for (int x = x_scan_start; x <= x_scan_end; x++)
        {
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                const float current_depth = current_attrib.inv_w;
                if (current_depth > get_z_buffer_at(x, y))
                {
                    // Call get_pixel_color with NULL texture and base_color
                    uint32_t pixel_color = get_pixel_color(current_shading, current_attrib, base_color, NULL, 0, 0);

                    if ((pixel_color >> 24) > 0) {
                        draw_pixel(x, y, pixel_color);
                        set_z_buffer_at(x, y, current_depth);
                    }
                }
            }
            // Increment horizontal interpolators
            current_attrib.inv_w += inv_w_step;
            // ... increment r/g/b/w and nx/ny/nz/w ...
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_attrib.nx_over_w += nx_step;
            current_attrib.ny_over_w += ny_step;
            current_attrib.nz_over_w += nz_step;

        }
    }
}

// --- New Perspective Rasterizer (Example: Flat Top Filled) ---
// Similar updates needed for fill_flat_top_triangle_perspective
static void fill_flat_top_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color) // Pass base color for solid fill
{
    // Similar implementation to fill_flat_bottom_triangle_perspective,
    // iterating from y = y2 down to y0, interpolating from pa2.
    const int window_width = get_window_width();
    const int window_height = get_window_height();
    shading_method current_shading = get_shading_method();

    float inv_slope_x_left = calculate_inverse_slope(x2, y2, x0, y0);
    float inv_slope_x_right = calculate_inverse_slope(x2, y2, x1, y1);

    const float y_height = (float)(y2 - y0);
    if (fabsf(y_height) < EPSILON) return;
    const float inv_y_height = 1.0f / y_height;

    for (int y = y2; y >= y0; y--) // Iterate upwards
    {
        const float t = (float)(y2 - y) * inv_y_height; // t=0 at y2, t=1 at y0
        brh_perspective_attribs attrib_left, attrib_right;

        // Interpolate ALL attributes from pa2 to pa0/pa1
        attrib_left.inv_w = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        attrib_left.r_over_w = interpolate_float(pa2.r_over_w, pa0.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa2.g_over_w, pa0.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa2.b_over_w, pa0.b_over_w, t);
        attrib_left.nx_over_w = interpolate_float(pa2.nx_over_w, pa0.nx_over_w, t);
        attrib_left.ny_over_w = interpolate_float(pa2.ny_over_w, pa0.ny_over_w, t);
        attrib_left.nz_over_w = interpolate_float(pa2.nz_over_w, pa0.nz_over_w, t);

        attrib_right.inv_w = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        attrib_right.r_over_w = interpolate_float(pa2.r_over_w, pa1.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa2.g_over_w, pa1.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa2.b_over_w, pa1.b_over_w, t);
        attrib_right.nx_over_w = interpolate_float(pa2.nx_over_w, pa1.nx_over_w, t);
        attrib_right.ny_over_w = interpolate_float(pa2.ny_over_w, pa1.ny_over_w, t);
        attrib_right.nz_over_w = interpolate_float(pa2.nz_over_w, pa1.nz_over_w, t);


        float current_edge_x_left_f = interpolate_float((float)x2, (float)x0, t);
        float current_edge_x_right_f = interpolate_float((float)x2, (float)x1, t);

        int x_scan_start = (int)roundf(current_edge_x_left_f);
        int x_scan_end = (int)roundf(current_edge_x_right_f);

        if (x_scan_start > x_scan_end) {
            swap_int(&x_scan_start, &x_scan_end);
            swap_perspective_attribs(&attrib_left, &attrib_right);
        }

        const float x_scan_width = (float)(x_scan_end - x_scan_start);
        float inv_w_step = 0, r_step = 0, g_step = 0, b_step = 0;
        float nx_step = 0, ny_step = 0, nz_step = 0;

        if (fabsf(x_scan_width) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            nx_step = (attrib_right.nx_over_w - attrib_left.nx_over_w) * inv_x_scan_width;
            ny_step = (attrib_right.ny_over_w - attrib_left.ny_over_w) * inv_x_scan_width;
            nz_step = (attrib_right.nz_over_w - attrib_left.nz_over_w) * inv_x_scan_width;
        }

        brh_perspective_attribs current_attrib = attrib_left;

        for (int x = x_scan_start; x <= x_scan_end; x++)
        {
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                const float current_depth = current_attrib.inv_w;
                if (current_depth > get_z_buffer_at(x, y))
                {
                    uint32_t pixel_color = get_pixel_color(current_shading, current_attrib, base_color, NULL, 0, 0);
                    if ((pixel_color >> 24) > 0) {
                        draw_pixel(x, y, pixel_color);
                        set_z_buffer_at(x, y, current_depth);
                    }
                }
            }
            // Increment horizontal interpolators
            current_attrib.inv_w += inv_w_step;
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_attrib.nx_over_w += nx_step;
            current_attrib.ny_over_w += ny_step;
            current_attrib.nz_over_w += nz_step;
        }
    }
}

// --- Wireframe Drawing (unchanged) ---
void draw_triangle_outline(const brh_triangle* triangle, uint32_t color)
{
    // Cast floats to int for drawing lines
    int x0 = (int)triangle->vertices[0].position.x; int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x; int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x; int y2 = (int)triangle->vertices[2].position.y;

    draw_line_dda(x0, y0, x1, y1, color);
    draw_line_dda(x1, y1, x2, y2, color);
    draw_line_dda(x2, y2, x0, y0, color);
}


// --- High-level Triangle Drawing Functions (Updated) ---

// Prepare Perspective Attributes helper
static void prepare_perspective_attribs(brh_vertex v, brh_perspective_attribs* pa) {
    pa->inv_w = v.inv_w;

    // Texture coords (always prepare, used by texture rasterizers)
    pa->u_over_w = v.texel.u * v.inv_w;
    pa->v_over_w = v.texel.v * v.inv_w;

    // Gouraud Color (prepare if needed)
    if (get_shading_method() == SHADING_GOURAUD) {
        uint8_t r = (v.color >> 16) & 0xFF;
        uint8_t g = (v.color >> 8) & 0xFF;
        uint8_t b = v.color & 0xFF;
        pa->r_over_w = (float)r * v.inv_w;
        pa->g_over_w = (float)g * v.inv_w;
        pa->b_over_w = (float)b * v.inv_w;
    }
    else {
        pa->r_over_w = pa->g_over_w = pa->b_over_w = 0; // Not used
    }

    // Phong Normal (prepare if needed)
    if (get_shading_method() == SHADING_PHONG) {
        pa->nx_over_w = v.normal.x * v.inv_w;
        pa->ny_over_w = v.normal.y * v.inv_w;
        pa->nz_over_w = v.normal.z * v.inv_w;
    }
    else {
        pa->nx_over_w = pa->ny_over_w = pa->nz_over_w = 0; // Not used
    }
}

void draw_filled_triangle(brh_triangle* triangle, uint32_t color) // color is now base_color or flat_color
{
    // 1. Get screen coordinates and prepare attributes from ORIGINAL vertices
    int x0 = (int)triangle->vertices[0].position.x; int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x; int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x; int y2 = (int)triangle->vertices[2].position.y;

    brh_perspective_attribs pa0, pa1, pa2;
    prepare_perspective_attribs(triangle->vertices[0], &pa0);
    prepare_perspective_attribs(triangle->vertices[1], &pa1);
    prepare_perspective_attribs(triangle->vertices[2], &pa2);

    // 2. Sort screen coords AND perspective attributes together based on Y
    // Simple bubble sort (synchronizing screen coords and attributes)
    if (y0 > y1) {
        swap_int(&x0, &x1); swap_int(&y0, &y1);
        swap_perspective_attribs(&pa0, &pa1); // Swap attributes too
    }
    if (y1 > y2) {
        swap_int(&x1, &x2); swap_int(&y1, &y2);
        swap_perspective_attribs(&pa1, &pa2); // Swap attributes too
    }
    if (y0 > y1) {
        swap_int(&x0, &x1); swap_int(&y0, &y1);
        swap_perspective_attribs(&pa0, &pa1); // Swap attributes too
    }

    // Now x0,y0 corresponds to pa0, x1,y1 to pa1, x2,y2 to pa2, AND y0 <= y1 <= y2
    assert(y0 <= y1 && y1 <= y2);
    if (y2 == y0) return; // Degenerate triangle

    // Use the correct base color (passed in, could be original face color or pre-calculated flat color)
    uint32_t base_color = triangle->color;

    // 3. Flat-Top / Flat-Bottom Decomposition (using sorted coords and attributes)
    if (y1 == y2) { // Flat Bottom (V0 top, V1/V2 bottom)
        fill_flat_bottom_triangle_perspective(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_color);
    }
    else if (y0 == y1) { // Flat Top (V0/V1 top, V2 bottom)
        fill_flat_top_triangle_perspective(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_color);
    }
    else { // General triangle: Split into two
        int my = y1;
        int mx = (int)roundf(interpolate_x_from_y(x0, y0, x2, y2, my));

        // Interpolate perspective attributes for the midpoint M along edge 0->2
        float y_delta_total = (float)(y2 - y0);
        if (fabsf(y_delta_total) < EPSILON) return;
        float lerp_factor_y = (float)(y1 - y0) / y_delta_total;

        brh_perspective_attribs pam; // Midpoint attributes
        pam.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, lerp_factor_y);
        if (fabsf(pam.inv_w) < EPSILON) { pam.inv_w = EPSILON; }
        // Interpolate ALL other attributes
        pam.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, lerp_factor_y); // Not used for fill, but keep consistent
        pam.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, lerp_factor_y);
        pam.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, lerp_factor_y);
        pam.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, lerp_factor_y);
        pam.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, lerp_factor_y);
        pam.nx_over_w = interpolate_float(pa0.nx_over_w, pa2.nx_over_w, lerp_factor_y);
        pam.ny_over_w = interpolate_float(pa0.ny_over_w, pa2.ny_over_w, lerp_factor_y);
        pam.nz_over_w = interpolate_float(pa0.nz_over_w, pa2.nz_over_w, lerp_factor_y);

        // Draw top part (Flat Bottom: V0, V1, M)
        fill_flat_bottom_triangle_perspective(x0, y0, pa0, x1, y1, pa1, mx, my, pam, base_color);

        // Draw bottom part (Flat Top: V1, M, V2 or M, V1, V2 depending on which is left)
        if (x1 < mx) { // V1 is left, M is right
            fill_flat_top_triangle_perspective(x1, y1, pa1, mx, my, pam, x2, y2, pa2, base_color);
        }
        else { // M is left, V1 is right
            fill_flat_top_triangle_perspective(mx, my, pam, x1, y1, pa1, x2, y2, pa2, base_color);
        }
    }
}

void draw_textured_triangle(brh_triangle* triangle, brh_texture_handle texture_handle)
{
    // Retrieve texture data
    if (!texture_handle) {
        fprintf(stderr, "Error: Invalid texture handle passed to draw_textured_triangle.\n");
        return;
    }
    uint32_t* texture_data = get_texture_data(texture_handle);
    int texture_width = get_texture_width(texture_handle);
    int texture_height = get_texture_height(texture_handle);
    if (!texture_data || texture_width <= 0 || texture_height <= 0) {
        fprintf(stderr, "Error: Failed to retrieve texture data for drawing.\n");
        return;
    }

    // 1. Get screen coordinates and prepare attributes from ORIGINAL vertices
    int x0 = (int)triangle->vertices[0].position.x; int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x; int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x; int y2 = (int)triangle->vertices[2].position.y;

    brh_perspective_attribs pa0, pa1, pa2;
    prepare_perspective_attribs(triangle->vertices[0], &pa0);
    prepare_perspective_attribs(triangle->vertices[1], &pa1);
    prepare_perspective_attribs(triangle->vertices[2], &pa2);

    // 2. Sort screen coords AND perspective attributes together based on Y
    if (y0 > y1) {
        swap_int(&x0, &x1); swap_int(&y0, &y1);
        swap_perspective_attribs(&pa0, &pa1);
    }
    if (y1 > y2) {
        swap_int(&x1, &x2); swap_int(&y1, &y2);
        swap_perspective_attribs(&pa1, &pa2);
    }
    if (y0 > y1) {
        swap_int(&x0, &x1); swap_int(&y0, &y1);
        swap_perspective_attribs(&pa0, &pa1);
    }

    assert(y0 <= y1 && y1 <= y2);
    if (y2 == y0) return;

    // 3. Flat-Top / Flat-Bottom Decomposition
    if (y1 == y2) { // Flat Bottom
        texture_flat_bottom_triangle_perspective(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height);
    }
    else if (y0 == y1) { // Flat Top
        texture_flat_top_triangle_perspective(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height);
    }
    else { // General triangle: Split
        int my = y1;
        int mx = (int)roundf(interpolate_x_from_y(x0, y0, x2, y2, my));

        float y_delta_total = (float)(y2 - y0);
        if (fabsf(y_delta_total) < EPSILON) return;
        float lerp_factor_y = (float)(y1 - y0) / y_delta_total;

        brh_perspective_attribs pam; // Midpoint attributes
        pam.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, lerp_factor_y);
        if (fabsf(pam.inv_w) < EPSILON) { pam.inv_w = EPSILON; }
        // Interpolate ALL other attributes
        pam.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, lerp_factor_y);
        pam.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, lerp_factor_y);
        pam.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, lerp_factor_y);
        pam.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, lerp_factor_y);
        pam.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, lerp_factor_y);
        pam.nx_over_w = interpolate_float(pa0.nx_over_w, pa2.nx_over_w, lerp_factor_y);
        pam.ny_over_w = interpolate_float(pa0.ny_over_w, pa2.ny_over_w, lerp_factor_y);
        pam.nz_over_w = interpolate_float(pa0.nz_over_w, pa2.nz_over_w, lerp_factor_y);

        // Draw top part (Flat Bottom: V0, V1, M)
        texture_flat_bottom_triangle_perspective(x0, y0, pa0, x1, y1, pa1, mx, my, pam, texture_data, texture_width, texture_height);

        // Draw bottom part (Flat Top: V1, M, V2 or M, V1, V2)
        if (x1 < mx) { // V1 is left, M is right
            texture_flat_top_triangle_perspective(x1, y1, pa1, mx, my, pam, x2, y2, pa2, texture_data, texture_width, texture_height);
        }
        else { // M is left, V1 is right
            texture_flat_top_triangle_perspective(mx, my, pam, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height);
        }
    }
}


// --- Barycentric Coordinates (can be kept, but not used by current rasterizers) ---
brh_vector3 calculate_barycentic_coordinates(brh_vector2 p, brh_vector2 a, brh_vector2 b, brh_vector2 c)
{
    // ... implementation unchanged ...
    brh_vector2 ab = vec2_subtract(b, a);
    brh_vector2 ac = vec2_subtract(c, a);
    brh_vector2 ap = vec2_subtract(p, a);
    brh_vector2 pc = vec2_subtract(c, p);
    brh_vector2 pb = vec2_subtract(b, p);

    float area_parallelogram_abc = vec2_cross(ac, ab);

    if (fabs(area_parallelogram_abc) < EPSILON) {
        return (brh_vector3) { -1.0f, -1.0f, -1.0f }; // Indicate degenerate triangle
    }

    float inv_area = 1.0f / area_parallelogram_abc;
    float alpha = vec2_cross(pc, pb) * inv_area; // Weight for A
    float beta = vec2_cross(ac, ap) * inv_area;  // Weight for B
    float gamma = 1.0f - alpha - beta;           // Weight for C

    return (brh_vector3) { alpha, beta, gamma };
}

// --- Vertex Interpolation (can be kept for clipping or other uses) ---
brh_vertex interpolate_vertices(brh_vertex v0, brh_vertex v1, float t)
{
    brh_vertex result;
    // Interpolate position (clip space)
    result.position = vec4_lerp(v0.position, v1.position, t);

    // Interpolate texture coordinates
    result.texel.u = interpolate_float(v0.texel.u, v1.texel.u, t);
    result.texel.v = interpolate_float(v0.texel.v, v1.texel.v, t);

    // Interpolate normals (world space)
    result.normal = vec3_lerp(v0.normal, v1.normal, t);
    // Important: Re-normalize after interpolation if using for lighting later!
    // result.normal = vec3_unit_vector(result.normal); // Clipper might do this

    // Interpolate vertex color (Gouraud)
    result.color = interpolate_colors(v0.color, v1.color, t);

    // Interpolate inverse W (clip space)
    result.inv_w = interpolate_float(v0.inv_w, v1.inv_w, t);

    return result;
}