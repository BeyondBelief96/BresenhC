/**
 * @file brh_clipping.h
 * @brief Clip space triangle clipping for 3D rendering
 *
 * This module implements homogeneous clip space triangle clipping,
 * which eliminates or splits triangles that intersect the view frustum
 * boundaries. The implementation uses the Sutherland-Hodgman algorithm
 * adapted for 3D homogeneous coordinates.
 * 
 * We also provide a method to clip triangles against the view frustum planes
 * prior to the perspective divide step in camera space.
 */

#pragma once

#include "brh_triangle.h"
#include "brh_vector.h"
#include "brh_geometry.h"

 /** Maximum number of triangles that can result from clipping a single triangle */
#define MAX_CLIPPED_TRIANGLES 16

/**
 * @enum brh_clip_plane
 * @brief Enumerates the six clipping planes of the canonical view volume.
 *
 * In clip space, the visible volume is defined by:
 * - -w ≤ x ≤ w
 * - -w ≤ y ≤ w
 * - -w ≤ z ≤ w
 */
typedef enum {
    CLIP_LEFT = 0,   /**< Left plane (x = -w) */
    CLIP_RIGHT,      /**< Right plane (x = w) */
    CLIP_BOTTOM,     /**< Bottom plane (y = -w) */
    CLIP_TOP,        /**< Top plane (y = w) */
    CLIP_NEAR,       /**< Near plane (z = -w) */
    CLIP_FAR,        /**< Far plane (z = w) */
    CLIP_PLANE_COUNT /**< Total number of clipping planes */
} brh_clip_plane;

/*
* @enum brh_frustum_plane
* @brief Enumerates the six planes of the view frustum.
* 
* The view frustum is defined by the following planes:
* - Left plane
* - Right plane
* - Bottom plane
* - Top plane
* - Near plane
* - Far plane
* 
* The planes are used for frustum culling and clipping operations.
* Each plane is represented by a normal vector and a point on the plane.
*/
typedef enum {
	FRUSTUM_LEFT = 0,
	FRUSTUM_RIGHT,
	FRUSTUM_BOTTOM,
	FRUSTUM_TOP,
	FRUSTUM_NEAR,
	FRUSTUM_FAR,
} brh_frustum_plane;

/**
 * @brief Clips a triangle against all six clip space planes.
 *
 * This is the main entry point for triangle clipping. The function
 * processes the triangle against each clip plane in sequence, creating
 * intermediate results between planes. This implements a 3D variant
 * of the Sutherland-Hodgman polygon clipping algorithm.
 *
 * The algorithm uses two buffers to avoid allocating memory during clipping,
 * swapping between them for each stage of the clipping process.
 *
 * @param triangle Input triangle in clip space
 * @param output_triangles Array to store the resulting clipped triangles
 * @return Number of triangles after clipping (0 if fully clipped)
 */
int clip_triangle(brh_triangle* triangle, brh_triangle* output_triangles);

/*
* @brief Clips a polygon against the view frustum planes.
* 
* This function takes a polygon and clips it against the six planes of the view frustum.
* The polygon is modified in place to contain only the vertices that are inside the frustum.
* 
* The clipping is done using the Sutherland-Hodgman algorithm, which is adapted for 3D polygons.
* The function iterates through each edge of the polygon and checks if it is inside or outside
* the frustum planes. If an edge is partially inside, it is split to create new vertices.
* The resulting vertices are stored in the polygon's vertices array.
* 
* @param polygon Pointer to the polygon to be clipped
* 
* @return void
*/
void clip_polygon(brh_polygon* polygon);

/*
* @brief Breaks a polygon into triangles.
* 
* This function takes a polygon and breaks it into triangles using the
* triangle fan method. The polygon is assumed to be convex and the
* vertices are stored in a clockwise order.
* 
* The function creates a new array of triangles and returns 
* the number of triangles created. The triangles are stored in the
* output array.
* 
* @param polygon Pointer to the polygon to be broken into triangles
* @param num_triangles Pointer to an integer to store the number of triangles created
* 
* @return Pointer to an array of triangles created from the polygon
*/
brh_triangle* break_polygon_into_triangles(brh_polygon* polygon, int* num_triangles);

/*
* @brief Initializes the frustum planes based on the camera's field of view.
* 
* This function sets up the six planes of the view frustum based on the
* specified field of view in the y direction, and the near and far planes.
* The planes are used for frustum culling and clipping operations.
* 
* @param fov_y Field of view in the y direction (in degrees)
* @param near_plane Distance to the near clipping plane
* @param far_plane Distance to the far clipping plane
* 
* @return void
*/
void initialize_frustum_planes(float fov_y, float near_plane, float far_plane);

/* global variables */
extern brh_plane frustum_planes[6];