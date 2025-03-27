#pragma once

#include <stdint.h>
#include "brh_vector2.h"

/**
 * @struct brh_face
 * @brief Represents a face of a triangle mesh using vertex indexes.
 *
 * This structure defines a face in a triangle mesh by storing the indexes
 * of its three vertices. Each index corresponds to a vertex in the mesh's
 * vertex array.
 *
 * @var brh_face::a
 * Index of the first vertex of the face.
 *
 * @var brh_face::b
 * Index of the second vertex of the face.
 *
 * @var brh_face::c
 * Index of the third vertex of the face.
 */
typedef struct {
    int a;
    int b;
    int c;
} brh_face;

/**
 * @struct brh_triangle
 * @brief Represents a triangle in 2D space using three points.
 *
 * This structure defines a triangle by storing the coordinates of its
 * three vertices. Each vertex is represented by a `brh_vector2` structure,
 * which contains the x and y coordinates of the point.
 *
 * @var brh_triangle::points
 * Array of three `brh_vector2` structures representing the vertices of the triangle.
 */
typedef struct {
    brh_vector2 points[3];
} brh_triangle;

void draw_triangle_outline(brh_triangle, uint32_t color);
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);