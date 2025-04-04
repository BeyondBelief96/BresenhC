#pragma once

#include <stdint.h>
#include "brh_vector.h"

/**
 * @struct brh_triangle
 * @brief Represents a triangle in 2D space using three points.
 *
 * This structure defines a triangle by storing the coordinates of its
 * three vertices. Each vertex is represented by a `brh_vector2` structure,
 * which contains the x and y coordinates of the point.
 *
 * @var brh_triangle::points
 * Array of three `brh_vector2` structures representing the vertices of the triangle
 * 
 * @var brh_triangle::texels
 * Array of three `brh_texel` structures representing the texture coordinates (u,v) for each vertex of the triangle. This is used for texture mapping.
 * 
 * @var brh_triangle::color
 * The color of the triangle, represented as a 32-bit unsigned integer. This can be used for flat shading or solid color rendering.
 * 
 * @var brh_triangle::avg_depth
 * The average depth of the triangle, calculated as the average of the z-coordinates of its three vertices. This can be used for sorting triangles in the painter's algorithm for rendering.
 */
typedef struct {
    brh_vector2 points[3];
    brh_texel texels[3];
    uint32_t color;
    float avg_depth;
} brh_triangle;

/*
* @brief Draws the wireframe of a triangle.
* 
* This function draws the wireframe of a triangle using the Bresenham's line algorithm.
* 
* @param triangle The triangle to draw.
* @param color The color of the triangle.
* 
* @return void
*/
void draw_triangle_outline(brh_triangle, uint32_t color);

/*
* @brief Draws a filled triangle.
* 
* This function draws a filled triangle using the scanline algorithm.
* 
* @param x0 The x coordinate of the first vertex.
* @param y0 The y coordinate of the first vertex.
* @param x1 The x coordinate of the second vertex.
* @param y1 The y coordinate of the second vertex.
* @parm x2 The x coordinate of the third vertex.
* @param y2 The y coordinate of the third vertex.
* @param color The color of the triangle.
* 
* @return void
*/
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);