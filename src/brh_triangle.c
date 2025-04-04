#include <stdio.h>
#include <assert.h>
#include "brh_triangle.h"
#include "math_utils.h"
#include "display.h"


///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom
///////////////////////////////////////////////////////////////////////////////
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
	float inv_slope_1 = calculate_inverse_slope(x0, y0, x1, y1);
	float inv_slope_2 = calculate_inverse_slope(x0, y0, x2, y2);

    // Start x_start and x_end from the top vertex (x0,y0)
    float x_start = (float) x0;
    float x_end = (float) x0;

    // Loop all the scanlines from top to bottom
    for (int y = y0; y <= y2; y++) {
        draw_horizontal_line((int) x_start, (int) x_end, (int) y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top
///////////////////////////////////////////////////////////////////////////////
//
//  (x0,y0)------(x1,y1)
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
	float inv_slope_1 = calculate_inverse_slope(x2, y2, x0, y0);
	float inv_slope_2 = calculate_inverse_slope(x2, y2, x1, y1);

    // Start x_start and x_end from the bottom vertex (x2,y2)
    float x_start = (float) x2;
    float x_end = (float) x2;

    // Loop all the scanlines from bottom to top
    for (int y = y2; y >= y0; y--) {
        draw_horizontal_line((int) x_start, (int) x_end, (int) y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

void draw_triangle_outline(brh_triangle triangle, uint32_t color)
{
    draw_line_dda((int)triangle.vertices[0].position.x, (int)triangle.vertices[0].position.y, (int)triangle.vertices[1].position.x, (int)triangle.vertices[1].position.y, color);
    draw_line_dda((int)triangle.vertices[1].position.x, (int)triangle.vertices[1].position.y, (int)triangle.vertices[2].position.x, (int)triangle.vertices[2].position.y, color);
    draw_line_dda((int)triangle.vertices[2].position.x, (int)triangle.vertices[2].position.y, (int)triangle.vertices[0].position.x, (int)triangle.vertices[0].position.y, color);
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method
// We split the original triangle in two, half flat-bottom and half flat-top
///////////////////////////////////////////////////////////////////////////////
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)------(Mx,My)
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
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

    // Sort the vertices by y coordinate ascending to simplify the drawing
    if (y0 > y1)
    {
        swap_int(&x0, &x1);
        swap_int(&y0, &y1);
    }
    if (y1 > y2)
    {
        swap_int(&x1, &x2);
        swap_int(&y1, &y2);
    }
    if (y0 > y1)
    {
        swap_int(&x0, &x1);
        swap_int(&y0, &y1);
    }

    assert(y0 <= y1 && y1 <= y2);

    if ((int)y1 == (int)y2)
    {
        // Draw Flat Bottom Triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
        return;
    }
    else if ((int)y0 == (int)y1)
    {
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
        return;
    }
    else
    {
        // Calculate midpoint vertex of the longest side equal to y1
        int my = y1;
        int mx = (int)interpolate_x_from_y(x0, y0, x2, y2, y1);

        // Draw Flat Bottom Triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, mx, my, color);

        // Draw Flat Top Triangle
        fill_flat_top_triangle(x1, y1, mx, my, x2, y2, color);
    }
}



///////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle with the flat-top/flat-bottom method
// 
///////////////////////////////////////////////////////////////////////////////
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)         \ 
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(brh_triangle* triangle, uint32_t* texture)
{
    int x0 = (int)triangle->vertices[0].position.x;
    int y0 = (int)triangle->vertices[0].position.y;
    int x1 = (int)triangle->vertices[1].position.x;
    int y1 = (int)triangle->vertices[1].position.y;
    int x2 = (int)triangle->vertices[2].position.x;
    int y2 = (int)triangle->vertices[2].position.y;

    brh_texel t0 = triangle->vertices[0].texel;
    brh_texel t1 = triangle->vertices[1].texel;
    brh_texel t2 = triangle->vertices[2].texel;

    // Sort vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1)
    {
        SWAP(x0, x1, int);
        SWAP(y0, y1, int);
        SWAP(t0.u, t1.u, float);
        SWAP(t0.v, t1.v, float);
    }
    if (y1 > y2)
    {
        SWAP(x1, x2, int);
        SWAP(y1, y2, int);
        SWAP(t1.u, t2.u, float);
        SWAP(t1.v, t2.v, float);
    }
    if (y0 > y1)
    {
        SWAP(x0, x1, int);
        SWAP(y0, y1, int);
        SWAP(t0.u, t1.u, float);
        SWAP(t0.v, t1.v, float);
    }

    // Render the upper part of the triangle (flat bottom)
	float inv_slope_1 = calculate_inverse_slope(x0, y0, x1, y1);
	float inv_slope_2 = calculate_inverse_slope(x0, y0, x2, y2);

	float x_start = (float)x0;
	float x_end = (float)x0;
	// Start x_start and x_end from the top vertex (x0,y0)
	// Loop all the scanlines from top to bottom
	for (int y = y0; y <= y1; y++)
	{
        draw_horizontal_line((int)x_start, (int)x_end, y, 0xFFFF00FF);
		x_start += inv_slope_1;
		x_end += inv_slope_2;
	}
	// Render the lower part of the triangle (flat top)

	inv_slope_1 = calculate_inverse_slope(x2, y2, x0, y0);
	inv_slope_2 = calculate_inverse_slope(x2, y2, x1, y1);

    // Start x_start and x_end from the bottom vertex (x2,y2)
	x_start = (float)x2;
	x_end = (float)x2;

	// Loop all the scanlines from bottom to top
	for (int y = y2; y >= y1; y--)
	{
		draw_horizontal_line((int)x_start, (int)x_end, y, 0xFFFF00FF);
		x_start -= inv_slope_1;
		x_end -= inv_slope_2;
	}
}


 