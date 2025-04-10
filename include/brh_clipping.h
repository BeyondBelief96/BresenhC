#pragma once

#include "brh_vector.h"

typedef enum {
	LEFT_PLANE,
	RIGHT_PLANE,
	TOP_PLANE,
	BOTTOM_PLANE,
	NEAR_PLANE,
	FAR_PLANE
} brh_clipping_planes;

/*
* @brief Represents a plane in 3D space.
* A plane is defined by a point and a normal vector.
* The normal vector is perpendicular to the plane and defines its orientation.
* The point is a point on the plane.
*/
typedef struct {
	brh_vector3 point;
	brh_vector3 normal;
} brh_plane;


extern brh_plane clipping_planes[6];

/*
* @brief Initializes the frustum planes that will be used for clipping.
* This function calculates the six clipping planes of the frustum based on the
* provided field of view (FOV), near plane distance, and far plane distance.
* 
* @param fov The field of view angle in degrees.
* @param near_plane The distance to the near clipping plane.
* @param far_plane The distance to the far clipping plane.
* 
* @return void
*/
void initialize_frustum_clipping_planes(float fov, float near_plane, float far_plane);

/*
* @brief Finds the intersection point of a line and a plane.
* 
* This function calculates the intersection point of a line defined by two points
* (line_start and line_end) and a plane defined by a point and a normal vector.
* The intersection point is returned as a brh_vector3 structure.
* 
* @param line_start The starting point of the line.
* @param line_end The ending point of the line.
* @param plane The plane defined by a point and a normal vector.
* 
* @return The intersection point of the line and the plane as a brh_vector3 structure.
*/
brh_vector3 find_line_plane_intersection(brh_vector3 line_start, brh_vector3 line_end, brh_plane plane);

