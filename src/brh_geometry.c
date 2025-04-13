#include "brh_geometry.h"
#include "math_utils.h"

brh_polygon create_polygon_from_triangle(brh_vector3 v0, brh_vector3 v1, brh_vector3 v2)
{
	brh_polygon polygon = {
		.num_vertices = 3,
		.vertices = { v0, v1, v2 }
	};

	return polygon;
}

brh_vector3 find_line_plane_intersection(brh_vector3 line_start, brh_vector3 line_end, brh_plane plane)
{
	brh_vector3 q1_p = vec3_subtract(line_start, plane.point);
	brh_vector3 q2_p = vec3_subtract(line_end, plane.point);
	float d1 = vec3_dot(q1_p, plane.normal);
	float d2 = vec3_dot(q2_p, plane.normal);
	float t = d1 / (d1 - d2);

	brh_vector3 intersection = {
		.x = line_start.x + t * (line_end.x - line_start.x),
		.y = line_start.y + t * (line_end.y - line_start.y),
		.z = line_start.z + t * (line_end.z - line_start.z)
	};

	return intersection;
}