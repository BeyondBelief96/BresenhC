#pragma once

#include <stdint.h>
#include "brh_vector.h"
#include "brh_texture.h"

/**
* @struct brh_vertex
* @brief Represents a vertex in 3D space with position, texture coordinates, and normal vector.
*
* This structure defines a vertex in 3D space by storing its position, texture coordinates,
* and normal vector. The position is represented by a `brh_vector2` structure, which contains
* the x and y coordinates of the vertex. The texture coordinates are represented by a `brh_texel`
* structure, which contains the u and v coordinates for texture mapping. The normal vector is
* represented by a `brh_vector3` structure, which contains the x, y, and z components of the normal.
*
* @var brh_vertex::position
* A `brh_vector2` structure representing the position of the vertex in 3D space.
*
* @var brh_vertex::texel
* A `brh_texel` structure representing the texture coordinates (u,v) for the vertex. This is used for texture mapping.
*
* @var brh_vertex::normal
* A `brh_vector3` structure representing the normal vector for the vertex. This is used for lighting calculations.
*/
typedef struct brh_vertex {
    brh_vector2 position;
	brh_texel texel;
    brh_vector3 normal;
} brh_vertex;

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
    brh_vertex vertices[3];
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
* @param triangle The triangle to draw.
* @param color The color of the triangle.
* 
* @return void
*/
void draw_filled_triangle(brh_triangle* triangle, uint32_t color);


void draw_textured_triangle(brh_triangle* triangle, uint32_t* texture);