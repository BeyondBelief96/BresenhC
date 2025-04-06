#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h> 
#include "brh_triangle.h"
#include "brh_vector.h" 
#include "brh_texture.h" 
#include "math_utils.h"
#include "display.h" 

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom (Solid Color - Keep for reference)
///////////////////////////////////////////////////////////////////////////////
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = calculate_inverse_slope(x0, y0, x1, y1);
    float inv_slope_2 = calculate_inverse_slope(x0, y0, x2, y2);

    // Start x_start and x_end from the top vertex (x0,y0)
    float x_start = (float)x0;
    float x_end = (float)x0;

    // Loop all the scanlines from top to bottom
    for (int y = y0; y <= y1; y++) { // Use y1 or y2, they are the same here
        draw_horizontal_line((int)x_start, (int)x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top (Solid Color - Keep for reference)
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = calculate_inverse_slope(x2, y2, x0, y0);
    float inv_slope_2 = calculate_inverse_slope(x2, y2, x1, y1);

    // Start x_start and x_end from the bottom vertex (x2,y2)
    float x_start = (float)x2;
    float x_end = (float)x2;

    // Loop all the scanlines from bottom to top
    for (int y = y2; y >= y0; y--) { // Use y0 or y1, they are the same here
        draw_horizontal_line((int)x_start, (int)x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle with a flat bottom
///////////////////////////////////////////////////////////////////////////////
//        (x0,y0) -- t0
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2) -- t1, t2
///////////////////////////////////////////////////////////////////////////////
void texture_flat_bottom_triangle(
    int x0, int y0, int x1, int y1, int x2, int y2,
    brh_texel t0, brh_texel t1, brh_texel t2,
    uint32_t* texture) {
    // Calculate inverse slopes for x interpolation
    float inv_slope_1 = calculate_inverse_slope(x0, y0, x1, y1);
    float inv_slope_2 = calculate_inverse_slope(x0, y0, x2, y2);

    float x_start = (float)x0;
    float x_end = (float)x0;

    // Define vertices as vectors for barycentric calculation
    brh_vector2 v0 = { (float)x0, (float)y0 };
    brh_vector2 v1 = { (float)x1, (float)y1 };
    brh_vector2 v2 = { (float)x2, (float)y2 };

    // Loop scanlines from top (y0) to bottom (y1=y2)
    for (int y = y0; y <= y1; y++) {
        int start_x = (int)roundf(x_start);
        int end_x = (int)roundf(x_end);

        // Ensure x_start is left of x_end
        if (start_x > end_x) {
            swap_int(&start_x, &end_x);
        }

        for (int x = start_x; x <= end_x; x++) {
            // Calculate barycentric coordinates for the current pixel (x, y)
            brh_vector2 p = { (float)x, (float)y };
            brh_vector3 weights = calculate_barycentic_coordinates(p, v0, v1, v2);
            float alpha = weights.x;
            float beta = weights.y;
            float gamma = weights.z;

            // Interpolate texture coordinates (u, v) using barycentric weights
            // Note: This is linear interpolation, not perspective-correct.
            // For perspective correct, you'd interpolate 1/w, u/w, v/w and divide.
            float interp_u = (alpha * t0.u) + (beta * t1.u) + (gamma * t2.u);
            float interp_v = (alpha * t0.v) + (beta * t1.v) + (gamma * t2.v);

            // Map interpolated UV coordinates to texture space
            // Use % for wrapping; ensure results are non-negative.
            int tex_x = (int)(interp_u * texture_width);
            int tex_y = (int)(interp_v * texture_height);

            // Clamp or wrap texture coordinates (using modulo for wrapping)
            tex_x = tex_x % texture_width;
            tex_y = tex_y % texture_height;
            if (tex_x < 0) tex_x += texture_width;
            if (tex_y < 0) tex_y += texture_height;


            // Sample the texture color
            // Make sure 'texture' points to the valid, loaded mesh_texture_data
            uint32_t texel_color = texture[tex_y * texture_width + tex_x];

            // Draw the pixel
            draw_pixel(x, y, texel_color);
        }

        // Increment x coordinates for the next scanline
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}


///////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle with a flat top
///////////////////////////////////////////////////////////////////////////////
//
//  (x0,y0)------(x1,y1) -- t0, t1
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2) -- t2
//
///////////////////////////////////////////////////////////////////////////////
void texture_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
    brh_texel t0, brh_texel t1, brh_texel t2,
    uint32_t* texture) {
    // Calculate inverse slopes for x interpolation (from bottom vertex V2)
    float inv_slope_1 = calculate_inverse_slope(x2, y2, x0, y0); // Edge 2-0
    float inv_slope_2 = calculate_inverse_slope(x2, y2, x1, y1); // Edge 2-1

    float x_start = (float)x2;
    float x_end = (float)x2;

    // Define vertices as vectors for barycentric calculation
    brh_vector2 v0 = { (float)x0, (float)y0 };
    brh_vector2 v1 = { (float)x1, (float)y1 };
    brh_vector2 v2 = { (float)x2, (float)y2 };

    // Loop scanlines from bottom (y2) up to top (y0=y1)
    for (int y = y2; y >= y0; y--) {
        int start_x = (int)roundf(x_start);
        int end_x = (int)roundf(x_end);

        // Ensure x_start is left of x_end
        if (start_x > end_x) {
            swap_int(&start_x, &end_x);
        }

        for (int x = start_x; x <= end_x; x++) {
            // Calculate barycentric coordinates for the current pixel (x, y)
            brh_vector2 p = { (float)x, (float)y };
            brh_vector3 weights = calculate_barycentic_coordinates(p, v0, v1, v2);
            float alpha = weights.x;
            float beta = weights.y;
            float gamma = weights.z;

            // Interpolate texture coordinates (u, v) using barycentric weights
            float interp_u = (alpha * t0.u) + (beta * t1.u) + (gamma * t2.u);
            float interp_v = (alpha * t0.v) + (beta * t1.v) + (gamma * t2.v);

            // Map interpolated UV coordinates to texture space
            int tex_x = (int)(interp_u * texture_width);
            int tex_y = (int)(interp_v * texture_height);

            // Clamp or wrap texture coordinates (using modulo for wrapping)
            tex_x = tex_x % texture_width;
            tex_y = tex_y % texture_height;
            if (tex_x < 0) tex_x += texture_width;
            if (tex_y < 0) tex_y += texture_height;

            // Sample the texture color
            uint32_t texel_color = texture[tex_y * texture_width + tex_x];

            // Draw the pixel
            draw_pixel(x, y, texel_color);
        }

        // Decrement x coordinates for the next scanline (moving upwards)
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
void draw_filled_triangle(brh_triangle* triangle, uint32_t color)
{
    // Extract the vertices from the triangle struct
    int x0 = (int)triangle->vertices[0].position.x;
    int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x;
    int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x;
    int y2 = (int)triangle->vertices[2].position.y;

    // Sort the vertices by y coordinate ascending (y0 <= y1 <= y2)
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); }
    if (y1 > y2) { swap_int(&x1, &x2); swap_int(&y1, &y2); }
    if (y0 > y1) { swap_int(&x0, &x1); swap_int(&y0, &y1); }

    assert(y0 <= y1 && y1 <= y2);

    // Check for degenerate triangle (all points on a line, effectively zero height)
    if (y2 == y0) return; // Can't draw if height is 0


    if (y1 == y2)
    {
        // Top vertex is v0, bottom edge is horizontal (v1, v2) -> Flat Bottom
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else if (y0 == y1)
    {
        // Top edge is horizontal (v0, v1), bottom vertex is v2 -> Flat Top
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else
    {
        // General triangle: Split into flat-bottom and flat-top
        int my = y1;
        // Interpolate Mx coord on the long edge V0-V2 at the level of My=Y1
        int mx = (int)interpolate_x_from_y(x0, y0, x2, y2, my);

        // Draw the top part (Flat Bottom)
        // Vertices: (x0,y0), (x1,y1), (mx,my)
        fill_flat_bottom_triangle(x0, y0, x1, y1, mx, my, color);

        // Draw the bottom part (Flat Top)
        // Vertices: (x1,y1), (mx,my), (x2,y2)  -- Need to pass in correct order for fill_flat_top
        // Order for flat top: Top-Left(x1,y1), Top-Right(mx,my), Bottom(x2,y2)
        // OR Top-Left(mx,my), Top-Right(x1,y1), Bottom(x2,y2) depending on which is left/right
        if (x1 < mx) {
            fill_flat_top_triangle(x1, y1, mx, my, x2, y2, color);
        }
        else {
            fill_flat_top_triangle(mx, my, x1, y1, x2, y2, color);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle using scanline rasterization and barycentric coordinates
///////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(brh_triangle* triangle, uint32_t* texture)
{
    // Ensure texture data is loaded (ideally happens once at startup)
    if (!texture) {
        fprintf(stderr, "Error: Texture data not loaded or passed incorrectly.\n");
        return;
    }

    // --- Vertex Data Extraction and Sorting ---
    int x0 = (int)triangle->vertices[0].position.x;
    int y0 = (int)triangle->vertices[0].position.y;
	int z0 = (int)triangle->vertices[0].position.z;
	int w0 = (int)triangle->vertices[0].position.w;
    brh_texel t0 = triangle->vertices[0].texel;

    int x1 = (int)triangle->vertices[1].position.x;
    int y1 = (int)triangle->vertices[1].position.y;
	int z1 = (int)triangle->vertices[1].position.z;
	int w1 = (int)triangle->vertices[1].position.w;
    brh_texel t1 = triangle->vertices[1].texel;

    int x2 = (int)triangle->vertices[2].position.x;
    int y2 = (int)triangle->vertices[2].position.y;
	int z2 = (int)triangle->vertices[2].position.z;
	int w2 = (int)triangle->vertices[2].position.w;
    brh_texel t2 = triangle->vertices[2].texel;


    // Sort vertices by y-coordinate ascending (y0 <= y1 <= y2)
    // Make sure to swap ALL associated data (x, y, t) together
    if (y0 > y1) {
        swap_int(&x0, &x1); swap_int(&y0, &y1);
		swap_int(&z0, &z1); swap_int(&w0, &w1);
        brh_texel temp_t = t0; t0 = t1; t1 = temp_t; 
    }
    if (y1 > y2) {
        swap_int(&x1, &x2); swap_int(&y1, &y2);
		swap_int(&z1, &z2); swap_int(&w1, &w2);
        brh_texel temp_t = t1; t1 = t2; t2 = temp_t; 
    }
    // Repeat the first comparison in case the swap changed the order
    if (y0 > y1) {
        swap_int(&x0, &x1); swap_int(&y0, &y1);
        swap_int(&z0, &z1); swap_int(&w0, &w1);
        brh_texel temp_t = t0; t0 = t1; t1 = temp_t; 
    }

    assert(y0 <= y1 && y1 <= y2);

    // Check for degenerate triangle (effectively zero height)
    if (y2 == y0) {
        return; // Cannot rasterize a horizontal line this way
    }

    // --- Triangle Rasterization ---

    if (y1 == y2) {
        // Top vertex is v0, bottom edge is horizontal (v1, v2) -> Flat Bottom case
        texture_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, t0, t1, t2, texture);
    }
    else if (y0 == y1) {
        // Top edge is horizontal (v0, v1), bottom vertex is v2 -> Flat Top case
        texture_flat_top_triangle(x0, y0, x1, y1, x2, y2, t0, t1, t2, texture);
    }
    else {
        // General triangle: Needs splitting
        // Calculate the midpoint vertex (Mx, My) on the edge V0-V2 at the same y-level as V1
        int my = y1;
        int mx = (int)interpolate_x_from_y(x0, y0, x2, y2, my);

        // Calculate the texture coordinates (Tx, Ty) for the midpoint M
        // Interpolation factor based on vertical position of y1 relative to y0 and y2
        float lerp_factor = (float)(y1 - y0) / (float)(y2 - y0);
        brh_texel tm; // Midpoint texture coordinates
        tm.u = interpolate_float(t0.u, t2.u, lerp_factor);
        tm.v = interpolate_float(t0.v, t2.v, lerp_factor);

        // Draw the top part (Flat Bottom Triangle: V0, V1, M)
        // Vertices: (x0,y0, t0), (x1,y1, t1), (mx,my, tm)
        texture_flat_bottom_triangle(x0, y0, x1, y1, mx, my, t0, t1, tm, texture);

        // Draw the bottom part (Flat Top Triangle: V1, M, V2)
        // Vertices: (x1,y1, t1), (mx,my, tm), (x2,y2, t2)
        // Pass vertices in correct order for flat-top: top-left, top-right, bottom
        // Top-left/Top-right depends on whether V1 is left or right of M
        if (x1 < mx) {
            // V1 is left of M
            texture_flat_top_triangle(x1, y1, mx, my, x2, y2, t1, tm, t2, texture);
        }
        else {
            // M is left of V1
            texture_flat_top_triangle(mx, my, x1, y1, x2, y2, tm, t1, t2, texture);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Calculate barycentric coordinates
// Checks if the point P is inside the triangle defined by A, B, C
// Returns barycentric weights (alpha, beta, gamma)
///////////////////////////////////////////////////////////////////////////////
brh_vector3 calculate_barycentic_coordinates(brh_vector2 p, brh_vector2 a, brh_vector2 b, brh_vector2 c)
{
    brh_vector2 ab = vec2_subtract(b, a); // Vector B-A
    brh_vector2 ac = vec2_subtract(c, a); // Vector C-A
    brh_vector2 ap = vec2_subtract(p, a); // Vector P-A
    brh_vector2 pc = vec2_subtract(c, p); // Vector C-P (Note: original code used p->c, fixed)
    brh_vector2 pb = vec2_subtract(b, p); // Vector B-P (Note: original code used p->b, fixed)

    // Calculate the area of the parallelogram spanned by vectors AC and AB
    // This area is twice the signed area of the triangle ABC
    float area_parallelogram_abc = vec2_cross(ac, ab); // AC x AB

    // Handle degenerate triangles (collinear vertices)
    // If the area is very close to zero, the triangle is degenerate.
    // Return invalid weights or handle as needed. For simplicity, return {0,0,0}.
    if (fabs(area_parallelogram_abc) < EPSILON) { 
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    // Calculate barycentric coordinates using signed areas (proportions)
    // Alpha (weight for vertex A) = Area(PBC) / Area(ABC)
    // Beta (weight for vertex B) = Area(APC) / Area(ABC)  -> Equivalent to Area(ACP) / Area(BCA) -> C-A x P-A / Area
    // Gamma (weight for vertex C) = Area(PAB) / Area(ABC) -> Equivalent to Area(APB) / Area(CAB) -> B-A x P-A / Area

    // Alpha = (PC x PB) / (AC x AB)
    float alpha = vec2_cross(pc, pb) / area_parallelogram_abc;

    // Beta = (AC x AP) / (AC x AB)
    float beta = vec2_cross(ac, ap) / area_parallelogram_abc;

    // Gamma = 1 - Alpha - Beta (or (AP x AB) / (AC x AB) )
    float gamma = 1.0f - alpha - beta;

    // Return the weights
    return (brh_vector3) { alpha, beta, gamma };
}