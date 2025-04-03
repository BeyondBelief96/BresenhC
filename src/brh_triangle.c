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
    draw_line_dda((int) triangle.points[0].x, (int)triangle.points[0].y, (int)triangle.points[1].x, (int) triangle.points[1].y, color);
    draw_line_dda((int) triangle.points[1].x, (int) triangle.points[1].y, (int) triangle.points[2].x, (int) triangle.points[2].y, color);
    draw_line_dda((int) triangle.points[2].x, (int) triangle.points[2].y, (int) triangle.points[0].x, (int) triangle.points[0].y, color);
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
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
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
        int mx = (int) interpolate_x_from_y(x0, y0, x2, y2, y1);

        // Draw Flat Bottom Triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, mx, my, color);

        // Draw Flat Top Triangle
        fill_flat_top_triangle(x1, y1, mx, my, x2, y2, color);
    }
}

brh_vector3 get_face_normal(brh_vector3 a, brh_vector3 b, brh_vector3 c)
{
	brh_vector3 ab = vec3_subtract(b, a);
	brh_vector3 ac = vec3_subtract(c, a);
	brh_vector3 normal = vec3_cross(ab, ac);
	vec3_normalize(&normal);
    return normal;
}
