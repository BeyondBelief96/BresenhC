#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "brh_triangle.h"
#include "brh_vector.h"
#include "math_utils.h"
#include "brh_display.h"
#include "brh_light.h"   

// --- Forward Declarations --- 
static void texture_flat_bottom_perspective_none(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, const uint32_t* texture, int tex_w, int tex_h, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void texture_flat_top_perspective_none(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, const uint32_t* texture, int tex_w, int tex_h, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void texture_flat_bottom_perspective_flat(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t flat_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void texture_flat_top_perspective_flat(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t flat_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void texture_flat_bottom_perspective_gouraud(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, const uint32_t* texture, int tex_w, int tex_h, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void texture_flat_top_perspective_gouraud(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, const uint32_t* texture, int tex_w, int tex_h, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
// static void texture_flat_bottom_perspective_phong(...); // Not implemented yet
// static void texture_flat_top_perspective_phong(...);   // Not implemented yet
static void fill_flat_bottom_perspective_none(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t base_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void fill_flat_top_perspective_none(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t base_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void fill_flat_bottom_perspective_flat(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t flat_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void fill_flat_top_perspective_flat(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t flat_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void fill_flat_bottom_perspective_gouraud(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t base_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
static void fill_flat_top_perspective_gouraud(int x0, int y0, brh_perspective_attribs pa0, int x1, int y1, brh_perspective_attribs pa1, int x2, int y2, brh_perspective_attribs pa2, uint32_t base_color, uint32_t* color_buffer, float* z_buffer, int win_w, int win_h);
// static void fill_flat_bottom_perspective_phong(...); // Not implemented yet
// static void fill_flat_top_perspective_phong(...);   // Not implemented yet


// --- Helper: Swap Perspective Attributes ---
void swap_perspective_attribs(brh_perspective_attribs* a, brh_perspective_attribs* b) {
    brh_perspective_attribs temp = *a;
    *a = *b;
    *b = temp;
}

// --- Helper: Prepare Perspective Attributes ---
static void prepare_perspective_attribs(brh_vertex v, brh_perspective_attribs* pa) {
    // Ensure inv_w is valid before multiplication
    float safe_inv_w = (fabsf(v.inv_w) < EPSILON) ? 0.0f : v.inv_w;
    pa->inv_w = safe_inv_w; // Store the potentially zero inv_w

    // Texture coords (needed for textured modes)
    pa->u_over_w = v.texel.u * safe_inv_w;
    pa->v_over_w = v.texel.v * safe_inv_w;

    // Gouraud Color (needed for Gouraud modes)
    shading_method current_shading = get_shading_method(); // Get shading mode once
    if (current_shading == SHADING_GOURAUD) {
        uint8_t r = (v.color >> 16) & 0xFF;
        uint8_t g = (v.color >> 8) & 0xFF;
        uint8_t b = v.color & 0xFF;
        pa->r_over_w = (float)r * safe_inv_w;
        pa->g_over_w = (float)g * safe_inv_w;
        pa->b_over_w = (float)b * safe_inv_w;
    }
    else {
        pa->r_over_w = pa->g_over_w = pa->b_over_w = 0;
    }

    // Phong Normal (needed for Phong modes)
    if (current_shading == SHADING_PHONG) {
        pa->nx_over_w = v.normal.x * safe_inv_w;
        pa->ny_over_w = v.normal.y * safe_inv_w;
        pa->nz_over_w = v.normal.z * safe_inv_w;
    }
    else {
        pa->nx_over_w = pa->ny_over_w = pa->nz_over_w = 0;
    }
}

//----------------------------------------------------------------------------
// Specialized Rasterizer Implementations (Completed for None, Flat, Gouraud)
//----------------------------------------------------------------------------

// --- Texture + None + Flat Bottom ---
static void texture_flat_bottom_perspective_none(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int tex_w, int tex_h,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y1 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y0; y <= y1; y++) {
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y - y0) * inv_y_height;

        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        attrib_left.u_over_w = interpolate_float(pa0.u_over_w, pa1.u_over_w, t);
        attrib_left.v_over_w = interpolate_float(pa0.v_over_w, pa1.v_over_w, t);

        attrib_right.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        attrib_right.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, t);
        attrib_right.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, t);

        float x_left_f = interpolate_float((float)x0, (float)x1, t);
        float x_right_f = interpolate_float((float)x0, (float)x2, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_perspective_attribs(&attrib_left, &attrib_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0, u_step = 0, v_step = 0;
        brh_perspective_attribs current_attrib = attrib_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            u_step = (attrib_right.u_over_w - attrib_left.u_over_w) * inv_x_scan_width;
            v_step = (attrib_right.v_over_w - attrib_left.v_over_w) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_attrib.inv_w += inv_w_step * offset;
                current_attrib.u_over_w += u_step * offset;
                current_attrib.v_over_w += v_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            const float current_depth = current_attrib.inv_w;
            if (current_depth > z_buffer[current_index]) {
                const float current_w = 1.0f / current_depth;
                const float u = current_attrib.u_over_w * current_w;
                const float v = current_attrib.v_over_w * current_w;
                int tx = (int)floorf(u * (float)tex_w);
                int ty = (int)floorf((1.0f - v) * (float)tex_h); // Flip V
                tx = ((tx % tex_w) + tex_w) % tex_w;
                ty = ((ty % tex_h) + tex_h) % tex_h;
                uint32_t pixel_color = texture[ty * tex_w + tx];

                if ((pixel_color >> 24) > 0) {
                    color_buffer[current_index] = pixel_color;
                    z_buffer[current_index] = current_depth;
                }
            }
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_step;
            current_attrib.v_over_w += v_step;
            current_index++;
        }
    }
}

// --- Texture + None + Flat Top ---
static void texture_flat_top_perspective_none(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int tex_w, int tex_h,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y2 - y0); // y2 is bottom, y0 is top
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y2; y >= y0; y--) { // Iterate upwards
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y2 - y) * inv_y_height; // t=0 at y2, t=1 at y0

        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        attrib_left.u_over_w = interpolate_float(pa2.u_over_w, pa0.u_over_w, t);
        attrib_left.v_over_w = interpolate_float(pa2.v_over_w, pa0.v_over_w, t);

        attrib_right.inv_w = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        attrib_right.u_over_w = interpolate_float(pa2.u_over_w, pa1.u_over_w, t);
        attrib_right.v_over_w = interpolate_float(pa2.v_over_w, pa1.v_over_w, t);

        float x_left_f = interpolate_float((float)x2, (float)x0, t);
        float x_right_f = interpolate_float((float)x2, (float)x1, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_perspective_attribs(&attrib_left, &attrib_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0, u_step = 0, v_step = 0;
        brh_perspective_attribs current_attrib = attrib_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            u_step = (attrib_right.u_over_w - attrib_left.u_over_w) * inv_x_scan_width;
            v_step = (attrib_right.v_over_w - attrib_left.v_over_w) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_attrib.inv_w += inv_w_step * offset;
                current_attrib.u_over_w += u_step * offset;
                current_attrib.v_over_w += v_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            const float current_depth = current_attrib.inv_w;
            if (current_depth > z_buffer[current_index]) {
                const float current_w = 1.0f / current_depth;
                const float u = current_attrib.u_over_w * current_w;
                const float v = current_attrib.v_over_w * current_w;
                int tx = (int)floorf(u * (float)tex_w);
                int ty = (int)floorf((1.0f - v) * (float)tex_h); // Flip V
                tx = ((tx % tex_w) + tex_w) % tex_w;
                ty = ((ty % tex_h) + tex_h) % tex_h;
                uint32_t pixel_color = texture[ty * tex_w + tx];

                if ((pixel_color >> 24) > 0) {
                    color_buffer[current_index] = pixel_color;
                    z_buffer[current_index] = current_depth;
                }
            }
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_step;
            current_attrib.v_over_w += v_step;
            current_index++;
        }
    }
}

// --- Texture + Flat + Flat Bottom ---
static void texture_flat_bottom_perspective_flat(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t flat_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y1 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y0; y <= y1; y++) {
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y - y0) * inv_y_height;

        float inv_w_left = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        float inv_w_right = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        float x_left_f = interpolate_float((float)x0, (float)x1, t);
        float x_right_f = interpolate_float((float)x0, (float)x2, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_float(&inv_w_left, &inv_w_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0;
        float current_inv_w = inv_w_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (inv_w_right - inv_w_left) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_inv_w += inv_w_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            if (current_inv_w > z_buffer[current_index]) {
                color_buffer[current_index] = flat_color; // Use the passed flat color
                z_buffer[current_index] = current_inv_w;
            }
            current_inv_w += inv_w_step;
            current_index++;
        }
    }
}

// --- Texture + Flat + Flat Top ---
static void texture_flat_top_perspective_flat(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t flat_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y2 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y2; y >= y0; y--) { // Iterate upwards
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y2 - y) * inv_y_height;

        float inv_w_left = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        float inv_w_right = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        float x_left_f = interpolate_float((float)x2, (float)x0, t);
        float x_right_f = interpolate_float((float)x2, (float)x1, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_float(&inv_w_left, &inv_w_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0;
        float current_inv_w = inv_w_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (inv_w_right - inv_w_left) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_inv_w += inv_w_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            if (current_inv_w > z_buffer[current_index]) {
                color_buffer[current_index] = flat_color; // Use the passed flat color
                z_buffer[current_index] = current_inv_w;
            }
            current_inv_w += inv_w_step;
            current_index++;
        }
    }
}

// --- Texture + Gouraud + Flat Bottom ---
static void texture_flat_bottom_perspective_gouraud(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int tex_w, int tex_h,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y1 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y0; y <= y1; y++) {
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y - y0) * inv_y_height;

        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        attrib_left.u_over_w = interpolate_float(pa0.u_over_w, pa1.u_over_w, t);
        attrib_left.v_over_w = interpolate_float(pa0.v_over_w, pa1.v_over_w, t);
        attrib_left.r_over_w = interpolate_float(pa0.r_over_w, pa1.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa0.g_over_w, pa1.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa0.b_over_w, pa1.b_over_w, t);

        attrib_right.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        attrib_right.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, t);
        attrib_right.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, t);
        attrib_right.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, t);

        float x_left_f = interpolate_float((float)x0, (float)x1, t);
        float x_right_f = interpolate_float((float)x0, (float)x2, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_perspective_attribs(&attrib_left, &attrib_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0, u_step = 0, v_step = 0, r_step = 0, g_step = 0, b_step = 0;
        brh_perspective_attribs current_attrib = attrib_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            u_step = (attrib_right.u_over_w - attrib_left.u_over_w) * inv_x_scan_width;
            v_step = (attrib_right.v_over_w - attrib_left.v_over_w) * inv_x_scan_width;
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_attrib.inv_w += inv_w_step * offset;
                current_attrib.u_over_w += u_step * offset;
                current_attrib.v_over_w += v_step * offset;
                current_attrib.r_over_w += r_step * offset;
                current_attrib.g_over_w += g_step * offset;
                current_attrib.b_over_w += b_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            const float current_depth = current_attrib.inv_w;
            if (current_depth > z_buffer[current_index]) {
                const float current_w = 1.0f / current_depth;
                // Texture
                const float u = current_attrib.u_over_w * current_w;
                const float v = current_attrib.v_over_w * current_w;
                int tx = (int)floorf(u * (float)tex_w);
                int ty = (int)floorf((1.0f - v) * (float)tex_h); // Flip V
                tx = ((tx % tex_w) + tex_w) % tex_w;
                ty = ((ty % tex_h) + tex_h) % tex_h;
                uint32_t base_color = texture[ty * tex_w + tx];
                // Gouraud
                uint8_t a_base = (base_color >> 24) & 0xFF;
                uint8_t r_base = (base_color >> 16) & 0xFF;
                uint8_t g_base = (base_color >> 8) & 0xFF;
                uint8_t b_base = base_color & 0xFF;
                float r_light = current_attrib.r_over_w * current_w;
                float g_light = current_attrib.g_over_w * current_w;
                float b_light = current_attrib.b_over_w * current_w;
                float r_intensity = MAX(0.0f, MIN(1.0f, r_light / 255.0f));
                float g_intensity = MAX(0.0f, MIN(1.0f, g_light / 255.0f));
                float b_intensity = MAX(0.0f, MIN(1.0f, b_light / 255.0f));
                uint8_t R = (uint8_t)((float)r_base * r_intensity);
                uint8_t G = (uint8_t)((float)g_base * g_intensity);
                uint8_t B = (uint8_t)((float)b_base * b_intensity);
                uint32_t final_color = ((uint32_t)a_base << 24) | ((uint32_t)R << 16) | ((uint32_t)G << 8) | B;

                if (a_base > 0) {
                    color_buffer[current_index] = final_color;
                    z_buffer[current_index] = current_depth;
                }
            }
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_step;
            current_attrib.v_over_w += v_step;
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_index++;
        }
    }
}

// --- Texture + Gouraud + Flat Top ---
static void texture_flat_top_perspective_gouraud(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture, int tex_w, int tex_h,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y2 - y0); // y2 bottom, y0 top
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y2; y >= y0; y--) { // Iterate upwards
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y2 - y) * inv_y_height; // t=0 at y2, t=1 at y0

        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        attrib_left.u_over_w = interpolate_float(pa2.u_over_w, pa0.u_over_w, t);
        attrib_left.v_over_w = interpolate_float(pa2.v_over_w, pa0.v_over_w, t);
        attrib_left.r_over_w = interpolate_float(pa2.r_over_w, pa0.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa2.g_over_w, pa0.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa2.b_over_w, pa0.b_over_w, t);

        attrib_right.inv_w = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        attrib_right.u_over_w = interpolate_float(pa2.u_over_w, pa1.u_over_w, t);
        attrib_right.v_over_w = interpolate_float(pa2.v_over_w, pa1.v_over_w, t);
        attrib_right.r_over_w = interpolate_float(pa2.r_over_w, pa1.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa2.g_over_w, pa1.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa2.b_over_w, pa1.b_over_w, t);

        float x_left_f = interpolate_float((float)x2, (float)x0, t);
        float x_right_f = interpolate_float((float)x2, (float)x1, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_perspective_attribs(&attrib_left, &attrib_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0, u_step = 0, v_step = 0, r_step = 0, g_step = 0, b_step = 0;
        brh_perspective_attribs current_attrib = attrib_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            u_step = (attrib_right.u_over_w - attrib_left.u_over_w) * inv_x_scan_width;
            v_step = (attrib_right.v_over_w - attrib_left.v_over_w) * inv_x_scan_width;
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_attrib.inv_w += inv_w_step * offset;
                current_attrib.u_over_w += u_step * offset;
                current_attrib.v_over_w += v_step * offset;
                current_attrib.r_over_w += r_step * offset;
                current_attrib.g_over_w += g_step * offset;
                current_attrib.b_over_w += b_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            const float current_depth = current_attrib.inv_w;
            if (current_depth > z_buffer[current_index]) {
                const float current_w = 1.0f / current_depth;
                // Texture
                const float u = current_attrib.u_over_w * current_w;
                const float v = current_attrib.v_over_w * current_w;
                int tx = (int)floorf(u * (float)tex_w);
                int ty = (int)floorf((1.0f - v) * (float)tex_h); // Flip V
                tx = ((tx % tex_w) + tex_w) % tex_w;
                ty = ((ty % tex_h) + tex_h) % tex_h;
                uint32_t base_color = texture[ty * tex_w + tx];
                // Gouraud
                uint8_t a_base = (base_color >> 24) & 0xFF;
                uint8_t r_base = (base_color >> 16) & 0xFF;
                uint8_t g_base = (base_color >> 8) & 0xFF;
                uint8_t b_base = base_color & 0xFF;
                float r_light = current_attrib.r_over_w * current_w;
                float g_light = current_attrib.g_over_w * current_w;
                float b_light = current_attrib.b_over_w * current_w;
                float r_intensity = MAX(0.0f, MIN(1.0f, r_light / 255.0f));
                float g_intensity = MAX(0.0f, MIN(1.0f, g_light / 255.0f));
                float b_intensity = MAX(0.0f, MIN(1.0f, b_light / 255.0f));
                uint8_t R = (uint8_t)((float)r_base * r_intensity);
                uint8_t G = (uint8_t)((float)g_base * g_intensity);
                uint8_t B = (uint8_t)((float)b_base * b_intensity);
                uint32_t final_color = ((uint32_t)a_base << 24) | ((uint32_t)R << 16) | ((uint32_t)G << 8) | B;

                if (a_base > 0) {
                    color_buffer[current_index] = final_color;
                    z_buffer[current_index] = current_depth;
                }
            }
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_step;
            current_attrib.v_over_w += v_step;
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_index++;
        }
    }
}

// --- Fill + None + Flat Bottom ---
static void fill_flat_bottom_perspective_none(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y1 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y0; y <= y1; y++) {
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y - y0) * inv_y_height;

        float inv_w_left = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        float inv_w_right = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        float x_left_f = interpolate_float((float)x0, (float)x1, t);
        float x_right_f = interpolate_float((float)x0, (float)x2, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_float(&inv_w_left, &inv_w_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0;
        float current_inv_w = inv_w_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (inv_w_right - inv_w_left) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_inv_w += inv_w_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            if (current_inv_w > z_buffer[current_index]) {
                color_buffer[current_index] = base_color;
                z_buffer[current_index] = current_inv_w;
            }
            current_inv_w += inv_w_step;
            current_index++;
        }
    }
}

// --- Fill + None + Flat Top ---
static void fill_flat_top_perspective_none(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y2 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;

    for (int y = y2; y >= y0; y--) { // Iterate upwards
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y2 - y) * inv_y_height;

        float inv_w_left = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        float inv_w_right = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        float x_left_f = interpolate_float((float)x2, (float)x0, t);
        float x_right_f = interpolate_float((float)x2, (float)x1, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_float(&inv_w_left, &inv_w_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0;
        float current_inv_w = inv_w_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (inv_w_right - inv_w_left) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_inv_w += inv_w_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            if (current_inv_w > z_buffer[current_index]) {
                color_buffer[current_index] = base_color;
                z_buffer[current_index] = current_inv_w;
            }
            current_inv_w += inv_w_step;
            current_index++;
        }
    }
}

// --- Fill + Flat + Flat Bottom ---
static void fill_flat_bottom_perspective_flat(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t flat_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    fill_flat_bottom_perspective_none(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, flat_color, color_buffer, z_buffer, win_w, win_h);
}

// --- Fill + Flat + Flat Top ---
static void fill_flat_top_perspective_flat(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t flat_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    fill_flat_top_perspective_none(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, flat_color, color_buffer, z_buffer, win_w, win_h);
}

// --- Fill + Gouraud + Flat Bottom ---
static void fill_flat_bottom_perspective_gouraud(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y1 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;
    const uint8_t a_base = (base_color >> 24) & 0xFF;

    for (int y = y0; y <= y1; y++) {
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y - y0) * inv_y_height;

        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        attrib_left.r_over_w = interpolate_float(pa0.r_over_w, pa1.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa0.g_over_w, pa1.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa0.b_over_w, pa1.b_over_w, t);

        attrib_right.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        attrib_right.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, t);

        float x_left_f = interpolate_float((float)x0, (float)x1, t);
        float x_right_f = interpolate_float((float)x0, (float)x2, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_perspective_attribs(&attrib_left, &attrib_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0, r_step = 0, g_step = 0, b_step = 0;
        brh_perspective_attribs current_attrib = attrib_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_attrib.inv_w += inv_w_step * offset;
                current_attrib.r_over_w += r_step * offset;
                current_attrib.g_over_w += g_step * offset;
                current_attrib.b_over_w += b_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            const float current_depth = current_attrib.inv_w;
            if (current_depth > z_buffer[current_index]) {
                const float current_w = 1.0f / current_depth;
                float r = current_attrib.r_over_w * current_w;
                float g = current_attrib.g_over_w * current_w;
                float b = current_attrib.b_over_w * current_w;
                uint8_t R = (uint8_t)MAX(0.0f, MIN(255.0f, r));
                uint8_t G = (uint8_t)MAX(0.0f, MIN(255.0f, g));
                uint8_t B = (uint8_t)MAX(0.0f, MIN(255.0f, b));
                uint32_t final_color = ((uint32_t)a_base << 24) | ((uint32_t)R << 16) | ((uint32_t)G << 8) | B;

                if (a_base > 0) {
                    color_buffer[current_index] = final_color;
                    z_buffer[current_index] = current_depth;
                }
            }
            current_attrib.inv_w += inv_w_step;
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_index++;
        }
    }
}

// --- Fill + Gouraud + Flat Top ---
static void fill_flat_top_perspective_gouraud(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    uint32_t base_color,
    uint32_t* color_buffer, float* z_buffer, int win_w, int win_h)
{
    const float y_delta = (float)(y2 - y0);
    if (fabsf(y_delta) < EPSILON) return;
    const float inv_y_height = 1.0f / y_delta;
    const uint8_t a_base = (base_color >> 24) & 0xFF;

    for (int y = y2; y >= y0; y--) { // Iterate upwards
        if (y < 0 || y >= win_h) continue;
        const float t = (float)(y2 - y) * inv_y_height;

        brh_perspective_attribs attrib_left, attrib_right;
        attrib_left.inv_w = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        attrib_left.r_over_w = interpolate_float(pa2.r_over_w, pa0.r_over_w, t);
        attrib_left.g_over_w = interpolate_float(pa2.g_over_w, pa0.g_over_w, t);
        attrib_left.b_over_w = interpolate_float(pa2.b_over_w, pa0.b_over_w, t);

        attrib_right.inv_w = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        attrib_right.r_over_w = interpolate_float(pa2.r_over_w, pa1.r_over_w, t);
        attrib_right.g_over_w = interpolate_float(pa2.g_over_w, pa1.g_over_w, t);
        attrib_right.b_over_w = interpolate_float(pa2.b_over_w, pa1.b_over_w, t);

        float x_left_f = interpolate_float((float)x2, (float)x0, t);
        float x_right_f = interpolate_float((float)x2, (float)x1, t);
        int x_start = (int)roundf(x_left_f);
        int x_end = (int)roundf(x_right_f);

        if (x_start > x_end) { swap_int(&x_start, &x_end); swap_perspective_attribs(&attrib_left, &attrib_right); }

        int x_start_clip = MAX(0, x_start);
        int x_end_clip = MIN(win_w - 1, x_end);

        const float x_scan_width_f = (float)(x_end - x_start);
        float inv_w_step = 0, r_step = 0, g_step = 0, b_step = 0;
        brh_perspective_attribs current_attrib = attrib_left;

        if (fabsf(x_scan_width_f) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width_f;
            inv_w_step = (attrib_right.inv_w - attrib_left.inv_w) * inv_x_scan_width;
            r_step = (attrib_right.r_over_w - attrib_left.r_over_w) * inv_x_scan_width;
            g_step = (attrib_right.g_over_w - attrib_left.g_over_w) * inv_x_scan_width;
            b_step = (attrib_right.b_over_w - attrib_left.b_over_w) * inv_x_scan_width;
            if (x_start_clip > x_start) {
                float offset = (float)(x_start_clip - x_start);
                current_attrib.inv_w += inv_w_step * offset;
                current_attrib.r_over_w += r_step * offset;
                current_attrib.g_over_w += g_step * offset;
                current_attrib.b_over_w += b_step * offset;
            }
        }

        int current_index = y * win_w + x_start_clip;
        for (int x = x_start_clip; x <= x_end_clip; x++) {
            const float current_depth = current_attrib.inv_w;
            if (current_depth > z_buffer[current_index]) {
                const float current_w = 1.0f / current_depth;
                float r = current_attrib.r_over_w * current_w;
                float g = current_attrib.g_over_w * current_w;
                float b = current_attrib.b_over_w * current_w;
                uint8_t R = (uint8_t)MAX(0.0f, MIN(255.0f, r));
                uint8_t G = (uint8_t)MAX(0.0f, MIN(255.0f, g));
                uint8_t B = (uint8_t)MAX(0.0f, MIN(255.0f, b));
                uint32_t final_color = ((uint32_t)a_base << 24) | ((uint32_t)R << 16) | ((uint32_t)G << 8) | B;

                if (a_base > 0) {
                    color_buffer[current_index] = final_color;
                    z_buffer[current_index] = current_depth;
                }
            }
            current_attrib.inv_w += inv_w_step;
            current_attrib.r_over_w += r_step;
            current_attrib.g_over_w += g_step;
            current_attrib.b_over_w += b_step;
            current_index++;
        }
    }
}


// --- TODO: Implement Phong Rasterizers ---
// ...

// --- Wireframe Drawing ---
void draw_triangle_outline(const brh_triangle* triangle, uint32_t color)
{
    int x0 = (int)triangle->vertices[0].position.x; int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x; int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x; int y2 = (int)triangle->vertices[2].position.y;

    // Consider optimizing draw_line_dda if performance critical
    draw_line_dda(x0, y0, x1, y1, color);
    draw_line_dda(x1, y1, x2, y2, color);
    draw_line_dda(x2, y2, x0, y0, color);
}


// --- High-level Triangle Drawing Functions (Dispatchers - UPDATED) ---

void draw_filled_triangle(brh_triangle* triangle, uint32_t color)
{
    // 1. Get buffer pointers and dimensions ONCE
    uint32_t* color_buffer = get_color_buffer_ptr();
    float* z_buffer = get_z_buffer_ptr();
    int win_w = get_window_width();
    int win_h = get_window_height();
    if (!color_buffer || !z_buffer || win_w <= 0 || win_h <= 0) return;

    // 2. Prepare attributes and sort vertices
    int x0 = (int)triangle->vertices[0].position.x; int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x; int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x; int y2 = (int)triangle->vertices[2].position.y;
    brh_perspective_attribs pa0, pa1, pa2;
    prepare_perspective_attribs(triangle->vertices[0], &pa0);
    prepare_perspective_attribs(triangle->vertices[1], &pa1);
    prepare_perspective_attribs(triangle->vertices[2], &pa2);
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); swap_perspective_attribs(&pa0, &pa1); }
    if (y1 > y2) { swap_int(&x1, &x2); swap_int(&y1, &y2); swap_perspective_attribs(&pa1, &pa2); }
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); swap_perspective_attribs(&pa0, &pa1); }
    assert(y0 <= y1 && y1 <= y2);
    if (y2 == y0) return;

    uint32_t base_or_flat_color = triangle->color;
    shading_method current_shading = get_shading_method();

    // 3. Split triangle and DISPATCH
    if (y1 == y2) { // Flat Bottom
        switch (current_shading) {
        case SHADING_NONE:    fill_flat_bottom_perspective_none(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_FLAT:    fill_flat_bottom_perspective_flat(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_GOURAUD: fill_flat_bottom_perspective_gouraud(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_PHONG:   /*fill_flat_bottom_perspective_phong(...)*/; break; // TODO
        }
    }
    else if (y0 == y1) { // Flat Top
        switch (current_shading) {
        case SHADING_NONE:    fill_flat_top_perspective_none(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_FLAT:    fill_flat_top_perspective_flat(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_GOURAUD: fill_flat_top_perspective_gouraud(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_PHONG:   /*fill_flat_top_perspective_phong(...)*/; break; // TODO
        }
    }
    else { // General triangle
        int my = y1;
        int mx = (int)roundf(interpolate_x_from_y(x0, y0, x2, y2, my));
        float y_delta_total = (float)(y2 - y0);
        if (fabsf(y_delta_total) < EPSILON) return;
        float lerp_factor_y = (float)(y1 - y0) / y_delta_total;
        brh_perspective_attribs pam;
        pam.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, lerp_factor_y);
        if (fabsf(pam.inv_w) < EPSILON) { pam.inv_w = EPSILON; }
        pam.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, lerp_factor_y);
        pam.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, lerp_factor_y);
        pam.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, lerp_factor_y);
        pam.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, lerp_factor_y);
        pam.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, lerp_factor_y);
        pam.nx_over_w = interpolate_float(pa0.nx_over_w, pa2.nx_over_w, lerp_factor_y);
        pam.ny_over_w = interpolate_float(pa0.ny_over_w, pa2.ny_over_w, lerp_factor_y);
        pam.nz_over_w = interpolate_float(pa0.nz_over_w, pa2.nz_over_w, lerp_factor_y);

        // Dispatch top part (Flat Bottom)
        switch (current_shading) {
        case SHADING_NONE:    fill_flat_bottom_perspective_none(x0, y0, pa0, x1, y1, pa1, mx, my, pam, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_FLAT:    fill_flat_bottom_perspective_flat(x0, y0, pa0, x1, y1, pa1, mx, my, pam, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_GOURAUD: fill_flat_bottom_perspective_gouraud(x0, y0, pa0, x1, y1, pa1, mx, my, pam, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_PHONG:   /*fill_flat_bottom_perspective_phong(...)*/; break; // TODO
        }

        // Dispatch bottom part (Flat Top)
        if (x1 < mx) {
            switch (current_shading) {
            case SHADING_NONE:    fill_flat_top_perspective_none(x1, y1, pa1, mx, my, pam, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_FLAT:    fill_flat_top_perspective_flat(x1, y1, pa1, mx, my, pam, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_GOURAUD: fill_flat_top_perspective_gouraud(x1, y1, pa1, mx, my, pam, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_PHONG:   /*fill_flat_top_perspective_phong(...)*/; break; // TODO
            }
        }
        else {
            switch (current_shading) {
            case SHADING_NONE:    fill_flat_top_perspective_none(mx, my, pam, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_FLAT:    fill_flat_top_perspective_flat(mx, my, pam, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_GOURAUD: fill_flat_top_perspective_gouraud(mx, my, pam, x1, y1, pa1, x2, y2, pa2, base_or_flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_PHONG:   /*fill_flat_top_perspective_phong(...)*/; break; // TODO
            }
        }
    }
}


void draw_textured_triangle(brh_triangle* triangle, brh_texture_handle texture_handle)
{
    // 1. Get buffer pointers, dimensions, and texture data
    uint32_t* color_buffer = get_color_buffer_ptr();
    float* z_buffer = get_z_buffer_ptr();
    int win_w = get_window_width();
    int win_h = get_window_height();
    if (!color_buffer || !z_buffer || win_w <= 0 || win_h <= 0) return;

    if (!texture_handle) { // Fallback to filled triangle if texture is missing
        fprintf(stderr, "Warning: Invalid texture handle in draw_textured_triangle. Falling back to filled.\n");
        draw_filled_triangle(triangle, triangle->color); // Use stored triangle color
        return;
    }
    uint32_t* texture_data = get_texture_data(texture_handle);
    int texture_width = get_texture_width(texture_handle);
    int texture_height = get_texture_height(texture_handle);
    if (!texture_data || texture_width <= 0 || texture_height <= 0) {
        fprintf(stderr, "Warning: Failed to get texture data in draw_textured_triangle. Falling back to filled.\n");
        draw_filled_triangle(triangle, triangle->color); // Use stored triangle color
        return;
    }

    // 2. Prepare attributes and sort vertices
    int x0 = (int)triangle->vertices[0].position.x; int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x; int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x; int y2 = (int)triangle->vertices[2].position.y;
    brh_perspective_attribs pa0, pa1, pa2;
    prepare_perspective_attribs(triangle->vertices[0], &pa0);
    prepare_perspective_attribs(triangle->vertices[1], &pa1);
    prepare_perspective_attribs(triangle->vertices[2], &pa2);
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); swap_perspective_attribs(&pa0, &pa1); }
    if (y1 > y2) { swap_int(&x1, &x2); swap_int(&y1, &y2); swap_perspective_attribs(&pa1, &pa2); }
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); swap_perspective_attribs(&pa0, &pa1); }
    assert(y0 <= y1 && y1 <= y2);
    if (y2 == y0) return;

    uint32_t flat_color = triangle->color; // Needed only for flat shading case
    shading_method current_shading = get_shading_method();

    // 3. Split triangle and DISPATCH
    if (y1 == y2) { // Flat Bottom
        switch (current_shading) {
        case SHADING_NONE:    texture_flat_bottom_perspective_none(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_FLAT:    texture_flat_bottom_perspective_flat(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_GOURAUD: texture_flat_bottom_perspective_gouraud(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_PHONG:   /*texture_flat_bottom_perspective_phong(...)*/; break; // TODO
        }
    }
    else if (y0 == y1) { // Flat Top
        switch (current_shading) {
        case SHADING_NONE:    texture_flat_top_perspective_none(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_FLAT:    texture_flat_top_perspective_flat(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_GOURAUD: texture_flat_top_perspective_gouraud(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_PHONG:   /*texture_flat_top_perspective_phong(...)*/; break; // TODO
        }
    }
    else { // General triangle
        int my = y1;
        int mx = (int)roundf(interpolate_x_from_y(x0, y0, x2, y2, my));
        float y_delta_total = (float)(y2 - y0);
        if (fabsf(y_delta_total) < EPSILON) return;
        float lerp_factor_y = (float)(y1 - y0) / y_delta_total;
        brh_perspective_attribs pam;
        pam.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, lerp_factor_y);
        if (fabsf(pam.inv_w) < EPSILON) { pam.inv_w = EPSILON; }
        pam.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, lerp_factor_y);
        pam.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, lerp_factor_y);
        pam.r_over_w = interpolate_float(pa0.r_over_w, pa2.r_over_w, lerp_factor_y);
        pam.g_over_w = interpolate_float(pa0.g_over_w, pa2.g_over_w, lerp_factor_y);
        pam.b_over_w = interpolate_float(pa0.b_over_w, pa2.b_over_w, lerp_factor_y);
        pam.nx_over_w = interpolate_float(pa0.nx_over_w, pa2.nx_over_w, lerp_factor_y);
        pam.ny_over_w = interpolate_float(pa0.ny_over_w, pa2.ny_over_w, lerp_factor_y);
        pam.nz_over_w = interpolate_float(pa0.nz_over_w, pa2.nz_over_w, lerp_factor_y);

        // Dispatch top part (Flat Bottom)
        switch (current_shading) {
        case SHADING_NONE:    texture_flat_bottom_perspective_none(x0, y0, pa0, x1, y1, pa1, mx, my, pam, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_FLAT:    texture_flat_bottom_perspective_flat(x0, y0, pa0, x1, y1, pa1, mx, my, pam, flat_color, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_GOURAUD: texture_flat_bottom_perspective_gouraud(x0, y0, pa0, x1, y1, pa1, mx, my, pam, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
        case SHADING_PHONG:   /*texture_flat_bottom_perspective_phong(...)*/; break; // TODO
        }

        // Dispatch bottom part (Flat Top)
        if (x1 < mx) {
            switch (current_shading) {
            case SHADING_NONE:    texture_flat_top_perspective_none(x1, y1, pa1, mx, my, pam, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_FLAT:    texture_flat_top_perspective_flat(x1, y1, pa1, mx, my, pam, x2, y2, pa2, flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_GOURAUD: texture_flat_top_perspective_gouraud(x1, y1, pa1, mx, my, pam, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_PHONG:   /*texture_flat_top_perspective_phong(...)*/; break; // TODO
            }
        }
        else {
            switch (current_shading) {
            case SHADING_NONE:    texture_flat_top_perspective_none(mx, my, pam, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_FLAT:    texture_flat_top_perspective_flat(mx, my, pam, x1, y1, pa1, x2, y2, pa2, flat_color, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_GOURAUD: texture_flat_top_perspective_gouraud(mx, my, pam, x1, y1, pa1, x2, y2, pa2, texture_data, texture_width, texture_height, color_buffer, z_buffer, win_w, win_h); break;
            case SHADING_PHONG:   /*texture_flat_top_perspective_phong(...)*/; break; // TODO
            }
        }
    }
}

// --- Vertex Interpolation (Removed - Not used by new rasterizers/clipping yet) ---
// brh_vertex interpolate_vertices(...)

// --- Barycentric Coordinates (Removed - Not used by new rasterizers) ---
// brh_vector3 calculate_barycentic_coordinates(...)