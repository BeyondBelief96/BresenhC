#pragma once

#include "brh_vector.h"

#define MAX_NUM_POLYGON_VERTICES 10

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

/*
* @brief Represents a polygon in 3D space.
* 
* A polygon is defined by a list of vertices.
* The number of vertices can vary, but it is limited to a maximum defined by MAX_NUM_POLYGON_VERTICES.
* The vertices are stored in a brh_vector3 array.
* 
* This polygon structure is used during our clipping algorithm to represent the polygonal shape
* of the object being rendered after clipping.
* 
* The vertices are stored in a clockwise order to ensure correct rendering.
* The number of vertices is stored in the num_vertices field.
* 
* @param vertices An array of brh_vector3 vertices representing the polygon's vertices.
* @param num_vertices The number of vertices in the polygon.
*/
typedef struct {
	brh_vector3 vertices[MAX_NUM_POLYGON_VERTICES];
	int num_vertices;
} brh_polygon;

/*
* @brief Creates a polygon from a triangle defined by three vertices.
* 
* This function takes three vertices (v0, v1, v2) and creates a polygon
* with those vertices. The polygon is defined in a clockwise order.
* The number of vertices in the polygon is set to 3.
* 
* @param v0 The first vertex of the triangle.
* @param v1 The second vertex of the triangle.
* @param v2 The third vertex of the triangle.
* 
* @return A brh_polygon structure representing the triangle as a polygon.
*/
brh_polygon create_polygon_from_triangle(brh_vector3 v0, brh_vector3 v1, brh_vector3 v2);

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


