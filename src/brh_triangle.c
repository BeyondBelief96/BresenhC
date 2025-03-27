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
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    assert(y0 <= y1); // Ensure y0 is less than or equal to y1
    assert(y1 == y2); // Ensure y1 is equal to y2 for flat bottom

    int scanline_x_start = x0;
    int scanline_x_end = x0;
    for (int scanline_y = y0; scanline_y <= y1; scanline_y++)
    {
        draw_horizontal_line(scanline_x_start, scanline_x_end, scanline_y, color);
        scanline_x_start = (int)interpolate_x_from_y(x0, y0, x1, y1, scanline_y);
        scanline_x_end = (int) interpolate_x_from_y(x0, y0, x2, y2, scanline_y);

        if (scanline_x_start > scanline_x_end)
        {
            swap_int(&scanline_x_start, &scanline_x_end);
        }

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
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    assert(y0 == y1); // Ensure y0 is equal to y1 for flat top
    assert(y2 >= y0); // Ensure y2 is greater than or equal to y0

    // Starting from top going up to the top.
    int scanline_x_start = x2;
    int scanline_x_end = x2;
    for (int scanline_y = y2; scanline_y >= y0; scanline_y--)
    {
        draw_horizontal_line(scanline_x_start, scanline_x_end, scanline_y, color);
        scanline_x_start = (int) interpolate_x_from_y(x2, y2, x0, y0, scanline_y);
        scanline_x_end = (int) interpolate_x_from_y(x2, y2, x1, y1, scanline_y);

		if (scanline_x_start > scanline_x_end)
		{
			swap_int(&scanline_x_start, &scanline_x_end);
		}
    }
}

void draw_triangle_outline(brh_triangle triangle, uint32_t color)
{
    draw_line_dda(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, color);
    draw_line_dda(triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, color);
    draw_line_dda(triangle.points[2].x, triangle.points[2].y, triangle.points[0].x, triangle.points[0].y, color);
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
