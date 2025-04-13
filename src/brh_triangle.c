#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "brh_triangle.h"
#include "brh_vector.h"
#include "brh_texture.h"
#include "math_utils.h"
#include "brh_display.h"

// --- Forward Declarations --- 
static void fill_flat_bottom_triangle(int x0, int y0, float inv_w0,
    int x1, int y1, float inv_w1,
    int x2, int y2, float inv_w2,
    uint32_t color);

static void fill_flat_top_triangle(int x0, int y0, float inv_w0,
    int x1, int y1, float inv_w1,
    int x2, int y2, float inv_w2,
    uint32_t color);

static void texture_flat_bottom_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture);

static void texture_flat_top_triangle_perspective(
    int x0, int y0, brh_perspective_attribs pa0,
    int x1, int y1, brh_perspective_attribs pa1,
    int x2, int y2, brh_perspective_attribs pa2,
    const uint32_t* texture);


// Helper function to swap perspective attributes
void swap_perspective_attribs(brh_perspective_attribs* a, brh_perspective_attribs* b) {
    brh_perspective_attribs temp = *a;
    *a = *b;
    *b = temp;
}


///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom (Solid Color - Kept for reference)
///////////////////////////////////////////////////////////////////////////////
void fill_flat_bottom_triangle(int x0, int y0, float inv_w0, int x1, int y1, float inv_w1, int x2, int y2, float inv_w2, uint32_t color) 
{
    const int window_width = get_window_width();
	const int window_height = get_window_height();

    // Calculate inverse screen slopes for X coordinates
    const float inv_slope_x_left = calculate_inverse_slope(x0, y0, x1, y1);
    const float inv_slope_x_right = calculate_inverse_slope(x0, y0, x2, y2);

    float current_edge_x_left = (float)x0;
    float current_edge_x_right = (float)x0;

    // Calculate vertical height for interpolation factor
    const float y_height = (float)(y1 - y0);
    if (fabsf(y_height) < EPSILON) return;
    const float inv_y_height = 1.0f / y_height;

    // Scan vertically from top to bottom
    for (int y = y0; y <= y1; y++)
    {
        // Calculate vertical interpolation factor 't'
        const float t = (float)(y - y0) * inv_y_height;

        // Interpolate 1/w for the left and right edges of the current scanline
        float scanline_inv_w_left = interpolate_float(inv_w0, inv_w1, t);
        float scanline_inv_w_right = interpolate_float(inv_w0, inv_w2, t);

        // Get integer screen X coordinates for the scanline span
        int x_scan_start = (int)roundf(current_edge_x_left);
        int x_scan_end = (int)roundf(current_edge_x_right);

        // Ensure left-to-right and swap interpolated 1/w if needed
        if (x_scan_start > x_scan_end) {
            swap_int(&x_scan_start, &x_scan_end);
            swap_float(&scanline_inv_w_left, &scanline_inv_w_right); // Need swap_float
        }

        // Calculate horizontal step for 1/w
        const float x_scan_width = (float)(x_scan_end - x_scan_start);
        float inv_w_step = 0.0f;

        if (fabsf(x_scan_width) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width;
            inv_w_step = (scanline_inv_w_right - scanline_inv_w_left) * inv_x_scan_width;
        }

        // Initialize 1/w interpolator for the current pixel
        float current_inv_w = scanline_inv_w_left;

        // Draw the horizontal span with Z-test
        for (int x = x_scan_start; x <= x_scan_end; x++)
        {
            // Add bounds checking to prevent out-of-bounds Z-buffer access
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                // Z-Buffer Test: Check if pixel is valid and closer than existing depth
            // Larger 1/w means closer.
                if (current_inv_w > get_z_buffer_at(x, y) && fabsf(current_inv_w) > EPSILON)
                {
                    draw_pixel(x, y, color);
					set_z_buffer_at(x, y, current_inv_w);
                }
                // Increment 1/w for the next pixel
                current_inv_w += inv_w_step;
            } 
        }

        // Update edge x-coordinates for the next scanline
        current_edge_x_left += inv_slope_x_left;
        current_edge_x_right += inv_slope_x_right;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top (Solid Color - Kept for reference)
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(int x0, int y0, float inv_w0, int x1, int y1, float inv_w1, int x2, int y2, float inv_w2, uint32_t color)
{
    const int window_width = get_window_width();
    const int window_height = get_window_height();
    // Calculate inverse screen slopes for X coordinates (from bottom vertex up)
    const float inv_slope_x_left = calculate_inverse_slope(x2, y2, x0, y0);
    const float inv_slope_x_right = calculate_inverse_slope(x2, y2, x1, y1);

    float current_edge_x_left = (float)x2;
    float current_edge_x_right = (float)x2;

    // Calculate vertical height for interpolation factor
    const float y_height = (float)(y2 - y0); // y2 > y0
    if (fabsf(y_height) < EPSILON) return;
    const float inv_y_height = 1.0f / y_height;

    // Scan vertically from bottom up to top
    for (int y = y2; y >= y0; y--)
    {
        // Calculate vertical interpolation factor 't' (0 at y2, 1 at y0)
        const float t = (float)(y2 - y) * inv_y_height;

        // Interpolate 1/w for the left (2->0) and right (2->1) edges
        float scanline_inv_w_left = interpolate_float(inv_w2, inv_w0, t);
        float scanline_inv_w_right = interpolate_float(inv_w2, inv_w1, t);

        // Get integer screen X coordinates
        int x_scan_start = (int)roundf(current_edge_x_left);
        int x_scan_end = (int)roundf(current_edge_x_right);

        // Ensure left-to-right and swap 1/w if needed
        if (x_scan_start > x_scan_end) {
            swap_int(&x_scan_start, &x_scan_end);
            swap_float(&scanline_inv_w_left, &scanline_inv_w_right); // Use swap_float
        }

        // Calculate horizontal step for 1/w
        const float x_scan_width = (float)(x_scan_end - x_scan_start);
        float inv_w_step = 0.0f;

        if (fabsf(x_scan_width) > EPSILON) {
            const float inv_x_scan_width = 1.0f / x_scan_width;
            inv_w_step = (scanline_inv_w_right - scanline_inv_w_left) * inv_x_scan_width;
        }

        // Initialize 1/w interpolator
        float current_inv_w = scanline_inv_w_left;

        // Draw the horizontal span with Z-test
        for (int x = x_scan_start; x <= x_scan_end; x++)
        {
            // Add bounds checking to prevent out-of-bounds Z-buffer access
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                // Z-Buffer Test
                if (current_inv_w > get_z_buffer_at(x, y) && fabsf(current_inv_w) > EPSILON)
                {
                    draw_pixel(x, y, color);
					set_z_buffer_at(x, y, current_inv_w);
                }
                // Increment 1/w
                current_inv_w += inv_w_step;
            }
        }

        // Update edge x-coordinates (moving upwards)
        current_edge_x_left -= inv_slope_x_left;
        current_edge_x_right -= inv_slope_x_right;
    }
}


///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method (Solid Color)
///////////////////////////////////////////////////////////////////////////////
void draw_filled_triangle(brh_triangle* triangle, uint32_t color)
{
    int x0 = (int)triangle->vertices[0].position.x;
    int y0 = (int)triangle->vertices[0].position.y;
    float inv_w0 = triangle->vertices[0].inv_w;

    int x1 = (int)triangle->vertices[1].position.x;
    int y1 = (int)triangle->vertices[1].position.y;
    float inv_w1 = triangle->vertices[1].inv_w;

    int x2 = (int)triangle->vertices[2].position.x;
    int y2 = (int)triangle->vertices[2].position.y;
    float inv_w2 = triangle->vertices[2].inv_w;


    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); swap_float(&inv_w0, &inv_w1); }
    if (y1 > y2) { swap_int(&x1, &x2); swap_int(&y1, &y2); swap_float(&inv_w1, &inv_w2); }
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); swap_float(&inv_w0, &inv_w1); }

    assert(y0 <= y1 && y1 <= y2);
    if (y2 == y0) return;

    if (y1 == y2) {
        fill_flat_bottom_triangle(x0, y0, inv_w0, x1, y1, inv_w1, x2, y2, inv_w2, color);
    }
    else if (y0 == y1) {
        fill_flat_top_triangle(x0, y0, inv_w0, x1, y1, inv_w1, x2, y2, inv_w2, color);
    }
    else {
        int my = y1;
        int mx = (int)roundf(interpolate_x_from_y(x0, y0, x2, y2, my));
        const float y_delta_total = (float)(y2 - y0);
        if (fabsf(y_delta_total) < EPSILON) return;
        const float vertical_interp_factor = (float)(my - y0) / y_delta_total;
		const float inv_w_midpoint = interpolate_float(inv_w0, inv_w2, vertical_interp_factor);

        fill_flat_bottom_triangle(x0, y0, inv_w0, x1, y1, inv_w1, mx, my, inv_w_midpoint, color);
        if (x1 < mx) {
            fill_flat_top_triangle(x1, y1, inv_w1, mx, my, inv_w_midpoint, x2, y2, inv_w2, color);
        }
        else {
            fill_flat_top_triangle(mx, my, inv_w_midpoint, x1, y1, inv_w1, x2, y2, inv_w2, color);
        }
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
    int x0, int y0, brh_perspective_attribs pa0, // Top vertex data
    int x1, int y1, brh_perspective_attribs pa1, // Bottom-left vertex data
    int x2, int y2, brh_perspective_attribs pa2, // Bottom-right vertex data
    const uint32_t* texture)                          // Texture data
{
    const int window_width = get_window_width();
    const int window_height = get_window_height();
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
        brh_perspective_attribs current_attrib_start;
        current_attrib_start.inv_w = interpolate_float(pa0.inv_w, pa1.inv_w, t);
        current_attrib_start.u_over_w = interpolate_float(pa0.u_over_w, pa1.u_over_w, t);
        current_attrib_start.v_over_w = interpolate_float(pa0.v_over_w, pa1.v_over_w, t);

        // Attributes for the ending point (right edge) of the current scanline
        brh_perspective_attribs current_attrib_end;
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
        brh_perspective_attribs current_attrib = current_attrib_start;

        // --- Pixel Loop ---
        // Iterate horizontally across the scanline from the left edge to the right edge.
        for (int x = current_x_start_scr; x <= current_x_end_scr; x++)
        {
            const float current_inv_w = current_attrib.inv_w; // Get 1/w for this pixel


            // Add bounds checking to prevent out-of-bounds Z-buffer access
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                // Z-Buffer Check & Perspective Safety Check
                // Check if 1/w is valid AND if it's closer than what's in the buffer
                // Larger 1/w means closer to the camera.
                if (current_inv_w > get_z_buffer_at(x, y) && fabsf(current_inv_w) > EPSILON)
                {
                    // --- Perspective Correction ---
                    const float current_w = 1.0f / current_inv_w;
                    const float current_u = current_attrib.u_over_w * current_w;
                    const float current_v = current_attrib.v_over_w * current_w;

                    // --- Texture Sampling ---
                    // Invert V coordinate during sampling if needed
                    int tex_x = (int)floorf(current_u * texture_width + EPSILON);
                    int tex_y = (int)floorf((1.0f - current_v) * texture_height + EPSILON); // Assuming V inversion needed here

                    tex_x = ((tex_x % texture_width) + texture_width) % texture_width;
                    tex_y = ((tex_y % texture_height) + texture_height) % texture_height;

                    const uint32_t texel_color = texture[tex_y * texture_width + tex_x];

                    // --- Draw Pixel ---
                    draw_pixel(x, y, texel_color);

                    // --- Update Z-Buffer ---
					set_z_buffer_at(x, y, current_inv_w); // Update depth buffer
                }

                // --- Increment Horizontal Interpolators (always happens) ---
                current_attrib.inv_w += inv_w_step;
                current_attrib.u_over_w += u_over_w_step;
                current_attrib.v_over_w += v_over_w_step;
            }
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
    int x0, int y0, brh_perspective_attribs pa0, // Top-left vertex data
    int x1, int y1, brh_perspective_attribs pa1, // Top-right vertex data
    int x2, int y2, brh_perspective_attribs pa2, // Bottom vertex data
    const uint32_t* texture)                          // Texture data
{
    const int window_width = get_window_width();
    const int window_height = get_window_height();
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
        brh_perspective_attribs current_attrib_start;
        current_attrib_start.inv_w = interpolate_float(pa2.inv_w, pa0.inv_w, t);
        current_attrib_start.u_over_w = interpolate_float(pa2.u_over_w, pa0.u_over_w, t);
        current_attrib_start.v_over_w = interpolate_float(pa2.v_over_w, pa0.v_over_w, t);

        // Attributes for the ending point (right edge 2->1) of the current scanline
        brh_perspective_attribs current_attrib_end;
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
        brh_perspective_attribs current_attrib = current_attrib_start;

        // --- Pixel Loop ---
        // Iterate horizontally across the scanline.
        for (int x = current_x_start_scr; x <= current_x_end_scr; x++)
        {
            const float current_inv_w = current_attrib.inv_w; // Get 1/w for this pixel


            // Add bounds checking to prevent out-of-bounds Z-buffer access
            if (x >= 0 && x < window_width && y >= 0 && y < window_height)
            {
                // Z-Buffer Check & Perspective Safety Check
            // Check if 1/w is valid AND if it's closer than what's in the buffer
            // Larger 1/w means closer to the camera.
                if (current_inv_w > get_z_buffer_at(x, y) && fabsf(current_inv_w) > EPSILON)
                {
                    // --- Perspective Correction ---
                    const float current_w = 1.0f / current_inv_w;
                    const float current_u = current_attrib.u_over_w * current_w;
                    const float current_v = current_attrib.v_over_w * current_w;

                    // --- Texture Sampling ---
                    // Invert V coordinate during sampling if needed
                    int tex_x = (int)floorf(current_u * texture_width + EPSILON);
                    int tex_y = (int)floorf((1.0f - current_v) * texture_height + EPSILON); // Assuming V inversion needed here

                    tex_x = ((tex_x % texture_width) + texture_width) % texture_width;
                    tex_y = ((tex_y % texture_height) + texture_height) % texture_height;

                    const uint32_t texel_color = texture[tex_y * texture_width + tex_x];

                    // --- Draw Pixel ---
                    draw_pixel(x, y, texel_color);

                    // --- Update Z-Buffer ---
					set_z_buffer_at(x, y, current_inv_w); // Update depth buffer
                }

                // --- Increment Horizontal Interpolators (always happens) ---
                current_attrib.inv_w += inv_w_step;
                current_attrib.u_over_w += u_over_w_step;
                current_attrib.v_over_w += v_over_w_step;
            }
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

    brh_perspective_attribs pa0, pa1, pa2;
    if (fabsf(inv_w0) < EPSILON || fabsf(inv_w1) < EPSILON || fabsf(inv_w2) < EPSILON)
    {
        return; 
    }
    pa0 = (brh_perspective_attribs){ .u_over_w = t0.u * inv_w0, .v_over_w = t0.v * inv_w0, .inv_w = inv_w0 };
    pa1 = (brh_perspective_attribs){ .u_over_w = t1.u * inv_w1, .v_over_w = t1.v * inv_w1, .inv_w = inv_w1 };
    pa2 = (brh_perspective_attribs){ .u_over_w = t2.u * inv_w2, .v_over_w = t2.v * inv_w2, .inv_w = inv_w2 };

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

        brh_perspective_attribs pam; // Midpoint perspective attributes
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

brh_vertex interpolate_vertices(brh_vertex v0, brh_vertex v1, float t)
{
    brh_vertex result;
    // Interpolate position
    result.position.x = v0.position.x + t * (v1.position.x - v0.position.x);
    result.position.y = v0.position.y + t * (v1.position.y - v0.position.y);
    result.position.z = v0.position.z + t * (v1.position.z - v0.position.z);
    result.position.w = v0.position.w + t * (v1.position.w - v0.position.w);

    // Interpolate texture coordinates
    result.texel.u = v0.texel.u + t * (v1.texel.u - v0.texel.u);
    result.texel.v = v0.texel.v + t * (v1.texel.v - v0.texel.v);

    // Interpolate normals
    result.normal.x = v0.normal.x + t * (v1.normal.x - v0.normal.x);
    result.normal.y = v0.normal.y + t * (v1.normal.y - v0.normal.y);
    result.normal.z = v0.normal.z + t * (v1.normal.z - v0.normal.z);

    // Interpolate inverse W (for perspective correction)
    result.inv_w = v0.inv_w + t * (v1.inv_w - v0.inv_w);

    return result;
    
}
