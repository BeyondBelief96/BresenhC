#pragma once

#include <stdbool.h>
#include "brh_geometry.h"
#include "brh_triangle.h"

// Maximum number of new triangles that can be created by clipping a single triangle
#define MAX_CLIPPED_TRIANGLES 16

typedef enum {
    CLIP_LEFT = 0,   // X < -W
    CLIP_RIGHT,      // X > W
    CLIP_BOTTOM,     // Y < -W
    CLIP_TOP,        // Y > W
    CLIP_NEAR,       // Z < -W
    CLIP_FAR,        // Z > W
    CLIP_PLANE_COUNT
} brh_clip_plane;

/*
* @brief Checks if a vertex is inside the clip space.
* 
* This function checks if a vertex is inside the clip space defined by the
* clipping planes. A vertex is considered inside if all its components are
* between -W and W, where W is the homogeneous coordinate of the vertex.
* 
* @param v The vertex to check.
* 
* @return true if the vertex is inside the clip space, false otherwise.
*/
bool is_vertex_inside_clipspace(brh_vector4 v);

/**
 * @brief Checks if a vertex is inside a specific clipping plane.
 *
 * This function checks if a vertex is inside the specified clipping plane.
 * The vertex is considered inside if it satisfies the condition defined by
 * the clipping plane.
 *
 * @param v The vertex to check.
 * @param plane The clipping plane to check against.
 * @return true if the vertex is inside the clipping plane, false otherwise.
 */
bool is_vertex_inside_plane(brh_vector4 v, brh_clip_plane plane);

/**
 * @brief Calculates the intersection parameter t where a line crosses a clip space plane.
 *
 * The function solves for t in the equation: v0 + t*(v1-v0) = intersection_point
 * For each clip plane, we use the specific equation of that plane:
 *
 * Examples:
 * - For left plane (x = -w):
 *   At intersection: v0.x + t*(v1.x-v0.x) = -(v0.w + t*(v1.w-v0.w))
 *   Solving for t: t = (-v0.w - v0.x) / ((v1.x - v0.x) + (v1.w - v0.w))
 *
 * - For right plane (x = w):
 *   At intersection: v0.x + t*(v1.x-v0.x) = v0.w + t*(v1.w-v0.w)
 *   Solving for t: t = (v0.w - v0.x) / ((v1.x - v0.x) - (v1.w - v0.w))
 *
 * The resulting t value (0 to 1) represents where along the line from v0 to v1
 * the intersection occurs, and is used to interpolate vertex attributes.
 *
 * @param v0 First vertex in homogeneous clip space coordinates
 * @param v1 Second vertex in homogeneous clip space coordinates
 * @param plane The clip plane to test against
 * @return Parameter t (0 to 1) where the line intersects the plane
 */
float get_line_plane_intersection_parameter(brh_vector4 v0, brh_vector4 v1, brh_clip_plane plane);

/*
* @brief Clips a triangle against a clipping plane.
* 
* This function clips a triangle against the clipping planes defined in the
* brh_clip_plane enum. The triangle is modified to fit within the clipping
* planes, and the output triangles are returned in the output parameter.
* 
* @param triangle The triangle to clip.
* @param output_triangles The output triangles after clipping.
* 
* @return The number of output triangles after clipping.
*/
int clip_triangle(brh_triangle* triangle, brh_triangle* output_triangles);
