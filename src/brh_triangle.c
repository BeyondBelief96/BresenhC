#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "brh_triangle.h"
#include "brh_vector.h"
#include "brh_texture.h"
#include "math_utils.h"
#include "display.h"

// Helper function to swap perspective attributes
void swap_perspective_attribs(brh_texture_persp_attribs* a, brh_texture_persp_attribs* b) {
    brh_texture_persp_attribs temp = *a;
    *a = *b;
    *b = temp;
}


///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom (Solid Color - Kept for reference)
///////////////////////////////////////////////////////////////////////////////
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    float inv_slope_1 = calculate_inverse_slope(x0, y0, x1, y1);
    float inv_slope_2 = calculate_inverse_slope(x0, y0, x2, y2);
    float x_start = (float)x0;
    float x_end = (float)x0;
    for (int y = y0; y <= y1; y++) {
        draw_horizontal_line((int)roundf(x_start), (int)roundf(x_end), y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top (Solid Color - Kept for reference)
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    float inv_slope_1 = calculate_inverse_slope(x2, y2, x0, y0);
    float inv_slope_2 = calculate_inverse_slope(x2, y2, x1, y1);
    float x_start = (float)x2;
    float x_end = (float)x2;
    for (int y = y2; y >= y0; y--) {
        draw_horizontal_line((int)roundf(x_start), (int)roundf(x_end), y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a PERSPECTIVE-CORRECT textured triangle with a flat bottom
// Vertices are sorted: y0 <= y1 == y2
// Top vertex: (x0, y0) [pa0]
// Bottom left: (x1, y1) [pa1]
// Bottom right: (x2, y2) [pa2]
//       (x0,y0) pa0
//         / \
//        /   \
// (x1,y1)-----(x2,y2) pa1, pa2
///////////////////////////////////////////////////////////////////////////////
void texture_flat_bottom_triangle_perspective(
    int x0, int y0, brh_texture_persp_attribs pa0, // Top vertex data
    int x1, int y1, brh_texture_persp_attribs pa1, // Bottom-left vertex data
    int x2, int y2, brh_texture_persp_attribs pa2, // Bottom-right vertex data
    uint32_t* texture)                          // Texture data
{
    // Calculate the inverse screen-space slopes (dx/dy) of the two non-horizontal edges.
    // These tell us how much the screen X coordinate changes for each step in Y.
    float inv_slope_1 = calculate_inverse_slope(x0, y0, x1, y1); // Left edge (0->1) slope
    float inv_slope_2 = calculate_inverse_slope(x0, y0, x2, y2); // Right edge (0->2) slope

    // Initialize the starting screen X coordinates for the scanline interpolation at the top vertex.
    float x_start = (float)x0;
    float x_end = (float)x0;

    // Calculate the vertical distance (height) of this triangle section.
    float y_delta = (float)(y1 - y0);
    // Avoid division by zero if the triangle is degenerate (height is near zero).
    if (fabsf(y_delta) < EPSILON) return;

    // Iterate scanlines vertically from the top vertex (y0) down to the bottom edge (y1).
    for (int y = y0; y <= y1; y++) {
        // --- Recalculate Edge Attributes ---
        // Calculate the interpolation factor 't' (from 0 to 1) representing the
        // vertical position of the current scanline 'y' relative to the top (y0)
        // and bottom (y1) of this triangle section. t = (current_y - start_y) / height
        float t = (float)(y - y0) / y_delta;

        // Linearly interpolate the perspective attributes (1/w, u/w, v/w) along the
        // left and right edges for the current scanline y.
        // This recalculation per scanline prevents the accumulation of floating-point
        // errors that would occur with incremental updates along the edges.
        // We interpolate these attributes because they are linear in screen space,
        // whereas raw u, v, and w are not.

        // Attributes for the starting point (left edge) of the current scanline
        brh_texture_persp_attribs current_attrib_start;
        current_attrib_start.inv_w = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        current_attrib_start.u_over_w = interpolate_float(pa0.u_over_w, pa1.u_over_w, t);
        current_attrib_start.v_over_w = interpolate_float(pa0.v_over_w, pa1.v_over_w, t);

        // Attributes for the ending point (right edge) of the current scanline
        brh_texture_persp_attribs current_attrib_end;
        current_attrib_end.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, t);
        current_attrib_end.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, t);
        current_attrib_end.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, t);

        // --- Horizontal Scanline Setup ---
        // Get the integer screen coordinates for the start and end of the horizontal span.
        int current_x_start_scr = (int)roundf(x_start);
        int current_x_end_scr = (int)roundf(x_end);

        // Ensure we iterate from left to right. If x_start > x_end, swap the
        // screen coordinates AND the corresponding interpolated edge attributes.
        if (current_x_start_scr > current_x_end_scr) {
            swap_int(&current_x_start_scr, &current_x_end_scr);
            swap_perspective_attribs(&current_attrib_start, &current_attrib_end);
        }

        // Calculate the change (step) in perspective attributes per pixel horizontally.
        float x_delta = (float)(current_x_end_scr - current_x_start_scr);
        float inv_w_step = 0, u_over_w_step = 0, v_over_w_step = 0;
        // Avoid division by zero for vertical lines or single-pixel-wide spans.
        if (fabsf(x_delta) > EPSILON) {
            float inv_x_delta = 1.0f / x_delta;
            inv_w_step = (current_attrib_end.inv_w - current_attrib_start.inv_w) * inv_x_delta;
            u_over_w_step = (current_attrib_end.u_over_w - current_attrib_start.u_over_w) * inv_x_delta;
            v_over_w_step = (current_attrib_end.v_over_w - current_attrib_start.v_over_w) * inv_x_delta;
        }

        // Initialize the interpolator for the first pixel of the scanline.
        brh_texture_persp_attribs current_attrib = current_attrib_start;

        // --- Pixel Loop ---
        // Iterate horizontally across the scanline from the left edge to the right edge.
        for (int x = current_x_start_scr; x <= current_x_end_scr; x++) {
            // Safety check: If 1/w is close to zero, w is very large (approaching infinity),
            // which happens near the camera's near plane or view frustum edges.
            // Trying to recover u/v would be numerically unstable or result in division by zero.
            // Proper clipping should ideally handle this before rasterization.
            if (fabsf(current_attrib.inv_w) < EPSILON) {
                // Skip this pixel but still increment attributes to proceed.
                current_attrib.inv_w += inv_w_step;
                current_attrib.u_over_w += u_over_w_step;
                current_attrib.v_over_w += v_over_w_step;
                continue;
            }

            // --- Perspective Correction ---
            // Recover the interpolated perspective-correct W for this pixel.
            float w = 1.0f / current_attrib.inv_w;
            // Recover the interpolated perspective-correct U and V coordinates by multiplying
            // the interpolated (u/w) and (v/w) by the recovered w.
            // Correct U = (U/W) * W
            // Correct V = (V/W) * W
            float interp_u = current_attrib.u_over_w * w;
            float interp_v = current_attrib.v_over_w * w;

            // --- Texture Sampling ---
            // Map the normalized texture coordinates (interp_u, interp_v) to integer
            // coordinates within the texture dimensions.
            // Use floorf and add a small epsilon to handle potential floating-point
            // inaccuracies near integer boundaries.
            int tex_x = (int)floorf(interp_u * texture_width + EPSILON);
            int tex_y = (int)floorf(interp_v * texture_height + EPSILON);

            // Wrap the texture coordinates if they fall outside the [0, width-1] or [0, height-1] range.
            // The double modulo ensures the result is always positive.
            tex_x = ((tex_x % texture_width) + texture_width) % texture_width;
            tex_y = ((tex_y % texture_height) + texture_height) % texture_height;

            // Get the color from the texture at the calculated coordinates.
            uint32_t texel_color = texture[tex_y * texture_width + tex_x];

            // --- Draw Pixel ---
            draw_pixel(x, y, texel_color);

            // --- Increment Horizontal Interpolators ---
            // Move the perspective attributes to the next pixel's position using the pre-calculated steps.
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_over_w_step;
            current_attrib.v_over_w += v_over_w_step;
        }

        // --- Update Edge X-Coordinates for Next Scanline ---
        // Increment the screen-space X coordinates along the edges using their slopes.
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}


///////////////////////////////////////////////////////////////////////////////
// Draw a PERSPECTIVE-CORRECT textured triangle with a flat top
// Vertices are sorted: y0 == y1 <= y2
// Top left: (x0, y0) [pa0]
// Top right: (x1, y1) [pa1]
// Bottom vertex: (x2, y2) [pa2]
//  (x0,y0)------(x1,y1) pa0, pa1
//      \         /
//       \       /
//        (x2,y2) pa2
///////////////////////////////////////////////////////////////////////////////
void texture_flat_top_triangle_perspective(
    int x0, int y0, brh_texture_persp_attribs pa0, // Top-left vertex data
    int x1, int y1, brh_texture_persp_attribs pa1, // Top-right vertex data
    int x2, int y2, brh_texture_persp_attribs pa2, // Bottom vertex data
    uint32_t* texture)                          // Texture data
{
    // Calculate the inverse screen-space slopes (dx/dy) of the two non-horizontal edges,
    // starting from the bottom vertex (x2, y2).
    float inv_slope_1 = calculate_inverse_slope(x2, y2, x0, y0); // Left edge (2->0) slope
    float inv_slope_2 = calculate_inverse_slope(x2, y2, x1, y1); // Right edge (2->1) slope

    // Initialize the starting screen X coordinates for the scanline interpolation at the bottom vertex.
    float x_start = (float)x2;
    float x_end = (float)x2;

    // Calculate the vertical distance (height) of this triangle section.
    float y_delta = (float)(y2 - y0); // y2 is the bottom, y0 is the top
    // Avoid division by zero if the triangle is degenerate.
    if (fabsf(y_delta) < EPSILON) return;

    // Iterate scanlines vertically from the bottom vertex (y2) up to the top edge (y0).
    for (int y = y2; y >= y0; y--) {
        // --- Recalculate Edge Attributes ---
        // Calculate the interpolation factor 't' (from 0 to 1) representing the
        // vertical position of the current scanline 'y' relative to the bottom (y2)
        // and top (y0) of this triangle section. t = (bottom_y - current_y) / height
        float t = (float)(y2 - y) / y_delta;

        // Linearly interpolate the perspective attributes (1/w, u/w, v/w) along the
        // left and right edges for the *current* scanline y. Interpolation starts
        // from the bottom vertex (pa2) towards the top vertices (pa0, pa1).
        // This prevents accumulation of floating-point errors along the edges.

        // Attributes for the starting point (left edge 2->0) of the current scanline
        brh_texture_persp_attribs current_attrib_start;
        current_attrib_start.inv_w = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        current_attrib_start.u_over_w = interpolate_float(pa2.u_over_w, pa0.u_over_w, t);
        current_attrib_start.v_over_w = interpolate_float(pa2.v_over_w, pa0.v_over_w, t);

        // Attributes for the ending point (right edge 2->1) of the current scanline
        brh_texture_persp_attribs current_attrib_end;
        current_attrib_end.inv_w = interpolate_float(pa2.inv_w, pa1.inv_w, t);
        current_attrib_end.u_over_w = interpolate_float(pa2.u_over_w, pa1.u_over_w, t);
        current_attrib_end.v_over_w = interpolate_float(pa2.v_over_w, pa1.v_over_w, t);

        // --- Horizontal Scanline Setup ---
        // Get the integer screen coordinates for the start and end of the horizontal span.
        int current_x_start_scr = (int)roundf(x_start);
        int current_x_end_scr = (int)roundf(x_end);

        // Ensure we iterate from left to right. Swap coordinates AND edge attributes if needed.
        if (current_x_start_scr > current_x_end_scr) {
            swap_int(&current_x_start_scr, &current_x_end_scr);
            swap_perspective_attribs(&current_attrib_start, &current_attrib_end);
        }

        // Calculate the change (step) in perspective attributes per pixel horizontally.
        float x_delta = (float)(current_x_end_scr - current_x_start_scr);
        float inv_w_step = 0, u_over_w_step = 0, v_over_w_step = 0;
        // Avoid division by zero for vertical lines or single-pixel-wide spans.
        if (fabsf(x_delta) > EPSILON) {
            float inv_x_delta = 1.0f / x_delta; // Use multiplication
            inv_w_step = (current_attrib_end.inv_w - current_attrib_start.inv_w) * inv_x_delta;
            u_over_w_step = (current_attrib_end.u_over_w - current_attrib_start.u_over_w) * inv_x_delta;
            v_over_w_step = (current_attrib_end.v_over_w - current_attrib_start.v_over_w) * inv_x_delta;
        }

        // Initialize the interpolator for the first pixel of the scanline.
        brh_texture_persp_attribs current_attrib = current_attrib_start;

        // --- Pixel Loop ---
        // Iterate horizontally across the scanline.
        for (int x = current_x_start_scr; x <= current_x_end_scr; x++) {
            // Safety check for 1/w near zero.
            if (fabsf(current_attrib.inv_w) < EPSILON) {
                current_attrib.inv_w += inv_w_step;
                current_attrib.u_over_w += u_over_w_step;
                current_attrib.v_over_w += v_over_w_step;
                continue;
            }

            // --- Perspective Correction ---
            // Recover W, U, V for this pixel.
            float w = 1.0f / current_attrib.inv_w;
            float interp_u = current_attrib.u_over_w * w;
            float interp_v = current_attrib.v_over_w * w;

            // --- Texture Sampling ---
            // Map normalized UV to texture space coordinates.
            int tex_x = (int)floorf(interp_u * texture_width + EPSILON);
            int tex_y = (int)floorf(interp_v * texture_height + EPSILON);

            // Wrap coordinates.
            tex_x = ((tex_x % texture_width) + texture_width) % texture_width;
            tex_y = ((tex_y % texture_height) + texture_height) % texture_height;

            // Sample texture.
            uint32_t texel_color = texture[tex_y * texture_width + tex_x];

            // --- Draw Pixel ---
            draw_pixel(x, y, texel_color);

            // --- Increment Horizontal Interpolators ---
            current_attrib.inv_w += inv_w_step;
            current_attrib.u_over_w += u_over_w_step;
            current_attrib.v_over_w += v_over_w_step;
        }

        // --- Update Edge X-Coordinates for Next Scanline ---
        // Decrement the screen-space X coordinates along the edges using their slopes,
        // as we are moving upwards (decreasing y).
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}


void draw_triangle_outline(const brh_triangle* triangle, uint32_t color)
{
    draw_line_dda((int)triangle->vertices[0].position.x, (int)triangle->vertices[0].position.y, (int)triangle->vertices[1].position.x, (int)triangle->vertices[1].position.y, color);
    draw_line_dda((int)triangle->vertices[1].position.x, (int)triangle->vertices[1].position.y, (int)triangle->vertices[2].position.x, (int)triangle->vertices[2].position.y, color);
    draw_line_dda((int)triangle->vertices[2].position.x, (int)triangle->vertices[2].position.y, (int)triangle->vertices[0].position.x, (int)triangle->vertices[0].position.y, color);
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method (Solid Color)
///////////////////////////////////////////////////////////////////////////////
// ... (draw_filled_triangle remains unchanged) ...
void draw_filled_triangle(brh_triangle* triangle, uint32_t color)
{
    // ... (existing implementation is fine) ...
    int x0 = (int)triangle->vertices[0].position.x;
    int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x;
    int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x;
    int y2 = (int)triangle->vertices[2].position.y;

    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); }
    if (y1 > y2) { swap_int(&x1, &x2); swap_int(&y1, &y2); }
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); }

    assert(y0 <= y1 && y1 <= y2);
    if (y2 == y0) return;

    if (y1 == y2) {
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else if (y0 == y1) {
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else {
        int my = y1;
        int mx = (int)interpolate_x_from_y(x0, y0, x2, y2, my);
        fill_flat_bottom_triangle(x0, y0, x1, y1, mx, my, color);
        if (x1 < mx) {
            fill_flat_top_triangle(x1, y1, mx, my, x2, y2, color);
        }
        else {
            fill_flat_top_triangle(mx, my, x1, y1, x2, y2, color);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Draw a PERSPECTIVE-CORRECT textured triangle using scanline rasterization
///////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(brh_triangle* triangle, uint32_t* texture)
{
    if (!texture) {
        fprintf(stderr, "Error: Texture data not loaded or passed incorrectly.\n");
        return;
    }

    int x0 = (int)triangle->vertices[0].position.x;
    int y0 = (int)triangle->vertices[0].position.y;
    float inv_w0 = triangle->vertices[0].inv_w;
    brh_texel t0 = triangle->vertices[0].texel;

    int x1 = (int)triangle->vertices[1].position.x;
    int y1 = (int)triangle->vertices[1].position.y;
    float inv_w1 = triangle->vertices[1].inv_w;
    brh_texel t1 = triangle->vertices[1].texel;

    int x2 = (int)triangle->vertices[2].position.x;
    int y2 = (int)triangle->vertices[2].position.y; 
    float inv_w2 = triangle->vertices[2].inv_w;
    brh_texel t2 = triangle->vertices[2].texel;

    brh_texture_persp_attribs pa0, pa1, pa2;
    if (fabsf(inv_w0) < EPSILON || fabsf(inv_w1) < EPSILON || fabsf(inv_w2) < EPSILON)
    {
        return; 
    }
    pa0 = (brh_texture_persp_attribs){ .u_over_w = t0.u * inv_w0, .v_over_w = t0.v * inv_w0, .inv_w = inv_w0 };
    pa1 = (brh_texture_persp_attribs){ .u_over_w = t1.u * inv_w1, .v_over_w = t1.v * inv_w1, .inv_w = inv_w1 };
    pa2 = (brh_texture_persp_attribs){ .u_over_w = t2.u * inv_w2, .v_over_w = t2.v * inv_w2, .inv_w = inv_w2 };

    if (y0 > y1)
    { 
        swap_int(&x0, &x1);
        swap_int(&y0, &y1);
        swap_perspective_attribs(&pa0, &pa1); 
    }
    if (y1 > y2) 
    {
        swap_int(&x1, &x2);
        swap_int(&y1, &y2); 
        swap_perspective_attribs(&pa1, &pa2);
    }
    if (y0 > y1) 
    {
        swap_int(&x0, &x1);
        swap_int(&y0, &y1);
        swap_perspective_attribs(&pa0, &pa1);
    }

    assert(y0 <= y1 && y1 <= y2);

    if (y2 == y0) { return; }

    // --- Triangle Rasterization using Flat-Top/Flat-Bottom Decomposition ---
    if (y1 == y2) {
        // Flat Bottom case
        texture_flat_bottom_triangle_perspective(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture);
    }
    else if (y0 == y1) {
        // Flat Top case
        texture_flat_top_triangle_perspective(x0, y0, pa0, x1, y1, pa1, x2, y2, pa2, texture);
    }
    else {
        // General triangle: Split
        int my = y1;
        // Use roundf for midpoint X for consistency with scanline start/end
        int mx = (int)roundf(interpolate_x_from_y(x0, y0, x2, y2, my));

        float y_delta_total = (float)(y2 - y0);
        if (fabsf(y_delta_total) < EPSILON) return;
        float lerp_factor_y = (float)(y1 - y0) / y_delta_total;

        brh_texture_persp_attribs pam; // Midpoint perspective attributes
        pam.inv_w = interpolate_float(pa0.inv_w, pa2.inv_w, lerp_factor_y);
        if (fabsf(pam.inv_w) < EPSILON) { return; } // Skip if midpoint unusable
        pam.u_over_w = interpolate_float(pa0.u_over_w, pa2.u_over_w, lerp_factor_y);
        pam.v_over_w = interpolate_float(pa0.v_over_w, pa2.v_over_w, lerp_factor_y);

        // Draw top part (Flat Bottom: V0, V1, M)
        texture_flat_bottom_triangle_perspective(x0, y0, pa0, x1, y1, pa1, mx, my, pam, texture);

        // Draw bottom part (Flat Top: V1, M, V2)
        if (x1 < mx) {
            texture_flat_top_triangle_perspective(x1, y1, pa1, mx, my, pam, x2, y2, pa2, texture);
        }
        else {
            texture_flat_top_triangle_perspective(mx, my, pam, x1, y1, pa1, x2, y2, pa2, texture);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Calculate barycentric coordinates (remains unchanged, not directly used
// by the new perspective scanline interpolation, but kept for potential other uses)
///////////////////////////////////////////////////////////////////////////////
brh_vector3 calculate_barycentic_coordinates(brh_vector2 p, brh_vector2 a, brh_vector2 b, brh_vector2 c)
{
    brh_vector2 ab = vec2_subtract(b, a);
    brh_vector2 ac = vec2_subtract(c, a);
    brh_vector2 ap = vec2_subtract(p, a);
    brh_vector2 pc = vec2_subtract(c, p);
    brh_vector2 pb = vec2_subtract(b, p);

    float area_parallelogram_abc = vec2_cross(ac, ab);

    if (fabs(area_parallelogram_abc) < EPSILON) {
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    float alpha = vec2_cross(pc, pb) / area_parallelogram_abc;
    float beta = vec2_cross(ac, ap) / area_parallelogram_abc;
    float gamma = 1.0f - alpha - beta;

    return (brh_vector3) { alpha, beta, gamma };
}