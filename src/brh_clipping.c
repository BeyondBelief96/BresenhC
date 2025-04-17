#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "brh_clipping.h"
#include "brh_light.h"
#include "brh_triangle.h"
#include "math_utils.h"

brh_plane frustum_planes[6];

/* function declarations*/
static bool is_vertex_inside_clipspace(brh_vector4 v);
static bool is_vertex_inside_plane(brh_vector4 v, brh_clip_plane plane);
static float get_line_plane_intersection_parameter(brh_vector4 v0, brh_vector4 v1, brh_clip_plane plane);
static int clip_triangle_against_plane(brh_triangle* triangle, brh_clip_plane plane, brh_triangle* output);
static void clip_polygon_against_frustum_plane(brh_polygon* polygon, brh_frustum_plane plane);
static brh_vertex interpolate_vertices(brh_vertex v0, brh_vertex v1, float t);

/* function definitions */

void initialize_frustum_planes(float fov_y, float near_plane, float far_plane) {
    frustum_planes[FRUSTUM_LEFT].point = vec3_create(0.0f, 0.0f, 0.0f);
	frustum_planes[FRUSTUM_LEFT].normal = vec3_create(cosf(fov_y / 2.0f), 0, sinf(fov_y / 2.0f));

	frustum_planes[FRUSTUM_RIGHT].point = vec3_create(0.0f, 0.0f, 0.0f);
	frustum_planes[FRUSTUM_RIGHT].normal = vec3_create(-cosf(fov_y / 2.0f), 0, sinf(fov_y / 2.0f));

	frustum_planes[FRUSTUM_TOP].point = vec3_create(0.0f, 0.0f, 0.0f);
	frustum_planes[FRUSTUM_TOP].normal = vec3_create(0, -cosf(fov_y / 2.0f), sinf(fov_y / 2.0f));

	frustum_planes[FRUSTUM_BOTTOM].point = vec3_create(0.0f, 0.0f, 0.0f);
	frustum_planes[FRUSTUM_BOTTOM].normal = vec3_create(0, cosf(fov_y / 2.0f), sinf(fov_y / 2.0f));

	frustum_planes[FRUSTUM_NEAR].point = vec3_create(0.0f, 0.0f, near_plane);
	frustum_planes[FRUSTUM_NEAR].normal = vec3_create(0, 0, 1);

	frustum_planes[FRUSTUM_FAR].point = vec3_create(0.0f, 0.0f, far_plane);
	frustum_planes[FRUSTUM_FAR].normal = vec3_create(0, 0, -1);
}


/**
 * @brief Checks if a vertex is inside the canonical view volume.
 *
 * A vertex is inside the clip space if it satisfies all these inequalities:
 * -w ≤ x ≤ w, -w ≤ y ≤ w, -w ≤ z ≤ w
 *
 * @param v Vertex in homogeneous clip space coordinates
 * @return true if vertex is inside clip space, false otherwise
 */
static bool is_vertex_inside_clipspace(brh_vector4 v)
{
    return (-v.w <= v.x && v.x <= v.w) &&
        (-v.w <= v.y && v.y <= v.w) &&
        (-v.w <= v.z && v.z <= v.w);
}

/**
 * @brief Checks if a vertex is inside a specific clip plane.
 *
 * Each clip plane is checked against the corresponding component:
 * - Left plane: x ≥ -w
 * - Right plane: x ≤ w
 * - Bottom plane: y ≥ -w
 * - Top plane: y ≤ w
 * - Near plane: z ≥ -w
 * - Far plane: z ≤ w
 *
 * @param v Vertex in homogeneous clip space coordinates
 * @param plane The clip plane to test against
 * @return true if vertex is inside the plane, false otherwise
 */
static bool is_vertex_inside_plane(brh_vector4 v, brh_clip_plane plane) {
    switch (plane)
    {
    case CLIP_LEFT:   return v.x >= -v.w;
    case CLIP_RIGHT:  return v.x <= v.w;
    case CLIP_BOTTOM: return v.y >= -v.w;
    case CLIP_TOP:    return v.y <= v.w;
    case CLIP_NEAR:   return v.z >= -v.w;
    case CLIP_FAR:    return v.z <= v.w;
    default:          return true;
    }
}

static brh_vertex interpolate_vertices(brh_vertex v0, brh_vertex v1, float t) {
    brh_vertex result;

    // Interpolate position (clip space)
    result.position = vec4_lerp(v0.position, v1.position, t);

    // Interpolate texture coordinates
    result.texel.u = interpolate_float(v0.texel.u, v1.texel.u, t);
    result.texel.v = interpolate_float(v0.texel.v, v1.texel.v, t);

    // Interpolate normals (world space)
    result.normal = vec3_lerp(v0.normal, v1.normal, t);

    // Interpolate vertex color (Gouraud shading)
    result.color = interpolate_colors(v0.color, v1.color, t);

    // Interpolate inverse W (clip space)
    result.inv_w = interpolate_float(v0.inv_w, v1.inv_w, t);

    return result;
}

/**
 * @brief Calculates the intersection parameter t where a line crosses a clip plane.
 *
 * The function solves for t in the equation: v0 + t*(v1-v0) = intersection_point
 * For each clip plane, the equation of the plane is used to find the intersection:
 *
 * For example, the left plane (x = -w) derivation:
 * At the intersection point, x = -w
 * v0.x + t*(v1.x-v0.x) = -(v0.w + t*(v1.w-v0.w))
 * Rearranging terms:
 * v0.x + t*(v1.x-v0.x) + v0.w + t*(v1.w-v0.w) = 0
 * t*((v1.x-v0.x) + (v1.w-v0.w)) = -v0.x - v0.w
 * t = (-v0.w - v0.x) / ((v1.x - v0.x) + (v1.w - v0.w))
 *
 * Similar derivations are used for other planes.
 *
 * @param v0 First vertex in homogeneous clip space coordinates
 * @param v1 Second vertex in homogeneous clip space coordinates
 * @param plane The clip plane to find intersection with
 * @return Parameter t (0 to 1) where the line intersects the plane
 */
static float get_line_plane_intersection_parameter(brh_vector4 v0, brh_vector4 v1, brh_clip_plane plane) {
    // Calculate intersection parameter t where v0 + t*(v1-v0) is on the plane
    float t = 0.0f;
    float denominator = 0.0f;  // For potential division by zero check

    switch (plane)
    {
    case CLIP_LEFT:
        // Equation: x = -w at intersection
        denominator = (v1.x - v0.x) + (v1.w - v0.w);
        if (fabsf(denominator) < EPSILON) return 0.0f;
        t = (-v0.w - v0.x) / denominator;
        break;
    case CLIP_RIGHT:
        // Equation: x = w at intersection
        denominator = (v1.x - v0.x) - (v1.w - v0.w);
        if (fabsf(denominator) < EPSILON) return 0.0f;
        t = (v0.w - v0.x) / denominator;
        break;
    case CLIP_BOTTOM:
        // Equation: y = -w at intersection
        denominator = (v1.y - v0.y) + (v1.w - v0.w);
        if (fabsf(denominator) < EPSILON) return 0.0f;
        t = (-v0.w - v0.y) / denominator;
        break;
    case CLIP_TOP:
        // Equation: y = w at intersection
        denominator = (v1.y - v0.y) - (v1.w - v0.w);
        if (fabsf(denominator) < EPSILON) return 0.0f;
        t = (v0.w - v0.y) / denominator;
        break;
    case CLIP_NEAR:
        // Equation: z = -w at intersection
        denominator = (v1.z - v0.z) + (v1.w - v0.w);
        if (fabsf(denominator) < EPSILON) return 0.0f;
        t = (-v0.w - v0.z) / denominator;
        break;
    case CLIP_FAR:
        // Equation: z = w at intersection
        denominator = (v1.z - v0.z) - (v1.w - v0.w);
        if (fabsf(denominator) < EPSILON) return 0.0f;
        t = (v0.w - v0.z) / denominator;
        break;
    default:
        break;
    }

    // Clamp t to range [0,1] to avoid precision issues
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return t;
}


brh_triangle* break_polygon_into_triangles(brh_polygon* polygon, int* num_triangles)
{
	brh_triangle* triangles = (brh_triangle*)malloc(sizeof(brh_triangle) * (polygon->num_vertices - 2));
	if (!triangles) {
		fprintf(stderr, "Error: Failed to allocate memory for triangles\n");
		return NULL;
	}

	for (int i = 0; i < polygon->num_vertices - 2; i++) {
        int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;

		triangles[i].vertices[0].position = vec4_from_vec3(polygon->vertices[index0]);
		triangles[i].vertices[1].position = vec4_from_vec3(polygon->vertices[index1]);
		triangles[i].vertices[2].position = vec4_from_vec3(polygon->vertices[index2]);
	}

	*num_triangles = polygon->num_vertices - 2;
	return triangles;
}

/*
* @brief Clips a polygon against a frustum plane.
 *
 * This function takes a polygon and clips it against a specified frustum plane.
 * The clipping is done using the Sutherland-Hodgman algorithm, which processes
 * each edge of the polygon and determines whether to keep or discard vertices
 * based on their position relative to the plane.
 *
 * @param polygon The polygon to clip
 * @param plane The frustum plane to clip against
*/
static void clip_polygon_against_frustum_plane(brh_polygon* polygon, brh_frustum_plane plane)
{
	brh_vector3 plane_point = frustum_planes[plane].point;
	brh_vector3 plane_normal = frustum_planes[plane].normal;

    brh_vector3 inside_vertices[MAX_NUM_POLYGON_VERTICES];
    int num_inside_vertices = 0;

    brh_vector3* current_vertex = &polygon->vertices[0];
    brh_vector3* previous_vertex = &polygon->vertices[polygon->num_vertices - 1];

	// Initializing the dot products for the first edge against the plane normal.
    float d1 = vec3_dot(vec3_subtract(*previous_vertex, plane_point), plane_normal);
	float d2 = vec3_dot(vec3_subtract(*current_vertex, plane_point), plane_normal);
    
	// Iterate through each edge of the polygon
    while (current_vertex != &polygon->vertices[polygon->num_vertices])
    {
		d2 = vec3_dot(vec3_subtract(*current_vertex, plane_point), plane_normal);
        
		// Check if the edge crosses the plane (negative product means one vertex is inside and the other is outside)
        if (d1 * d2 < 0)
        {
			// Calculate the intersection point
			brh_vector3 intersection_point = find_line_plane_intersection(*previous_vertex, *current_vertex, frustum_planes[plane]);
            inside_vertices[num_inside_vertices++] = intersection_point;
        }

        if (d2 > 0)
        {
			inside_vertices[num_inside_vertices++] = vec3_create(current_vertex->x, current_vertex->y, current_vertex->z);
        }


        d1 = d2;
        previous_vertex = current_vertex;
		current_vertex++;
    }

	// Update the polygon with the clipped vertices
	polygon->num_vertices = num_inside_vertices;
	for (int i = 0; i < num_inside_vertices; i++)
	{
		polygon->vertices[i] = inside_vertices[i];
	}
}

/**
 * @brief Clips a triangle against a single clip plane.
 *
 * Implements one stage of the Sutherland-Hodgman clipping algorithm.
 * The function processes each edge of the triangle and determines:
 * 1. If both vertices are inside: Keep the second vertex
 * 2. If both vertices are outside: Discard both
 * 3. If entering the boundary: Add the intersection point
 * 4. If exiting the boundary: Add the intersection point and the second vertex
 *
 * The output is a convex polygon that is triangulated into a fan.
 *
 * @param triangle The triangle to clip
 * @param plane The clip plane to clip against
 * @param output Array to store the resulting triangles
 * @return Number of output triangles after clipping
 */
static int clip_triangle_against_plane(brh_triangle* triangle, brh_clip_plane plane, brh_triangle* output) {
    brh_vertex* vertices = triangle->vertices;
    brh_vertex new_vertices[4];  // Maximum 4 vertices after clipping against one plane
    int num_vertices = 0;

    // Determine which vertices are inside the clip plane
    bool inside[3];
    for (int i = 0; i < 3; i++) {
        inside[i] = is_vertex_inside_plane(vertices[i].position, plane);
    }

    // Process each edge of the triangle using Sutherland-Hodgman algorithm
    for (int i = 0; i < 3; i++) {
        int j = (i + 1) % 3;  // Index of the next vertex

        // If current vertex is inside
        if (inside[i]) {
            // Add the current vertex to the output list
            new_vertices[num_vertices++] = vertices[i];
        }

        // If one vertex is inside and the other is outside (edge crosses the clip plane)
        if (inside[i] != inside[j]) {
            // Calculate intersection parameter
            float t = get_line_plane_intersection_parameter(
                vertices[i].position, vertices[j].position, plane);

            // Generate a new vertex at the intersection point
            new_vertices[num_vertices++] = interpolate_vertices(vertices[i], vertices[j], t);
        }
    }

    // Create output triangles using triangle fan (for convex polygons)
    int num_output_triangles = 0;

    if (num_vertices >= 3) {
        // First triangle uses the first three vertices
        output[num_output_triangles].vertices[0] = new_vertices[0];
        output[num_output_triangles].vertices[1] = new_vertices[1];
        output[num_output_triangles].vertices[2] = new_vertices[2];
        output[num_output_triangles].color = triangle->color;
        num_output_triangles++;

        // Additional triangles if we have more than 3 vertices
        // Each new triangle shares the first vertex and forms a fan
        for (int i = 3; i < num_vertices; i++) {
            output[num_output_triangles].vertices[0] = new_vertices[0];
            output[num_output_triangles].vertices[1] = new_vertices[i - 1];
            output[num_output_triangles].vertices[2] = new_vertices[i];
            output[num_output_triangles].color = triangle->color;
            num_output_triangles++;
        }
    }

    return num_output_triangles;
}


int clip_triangle(brh_triangle* triangle, brh_triangle* output_triangles) {
    // Create two arrays to hold triangles during the clipping process
    brh_triangle triangles_a[MAX_CLIPPED_TRIANGLES];
    brh_triangle triangles_b[MAX_CLIPPED_TRIANGLES];

    // Initialize the first buffer with the input triangle
    triangles_a[0] = *triangle;
    int num_triangles_a = 1;
    int num_triangles_b = 0;

    // Pointers to the current and next sets of triangles
    brh_triangle* current = triangles_a;
    brh_triangle* next = triangles_b;
    int* num_current = &num_triangles_a;
    int* num_next = &num_triangles_b;

    // Clip against each plane in sequence
    for (int plane = 0; plane < CLIP_PLANE_COUNT; plane++) {
        *num_next = 0;

        // Process each triangle in the current set
        for (int i = 0; i < *num_current; i++) {
            // Clip this triangle against the current plane
            int new_triangles = clip_triangle_against_plane(
                &current[i],
                (brh_clip_plane)plane,
                &next[*num_next]);

            *num_next += new_triangles;

            // Safety check to prevent buffer overflow
            if (*num_next > MAX_CLIPPED_TRIANGLES - 3) {
                fprintf(stderr, "Warning: Triangle clip buffer overflow\n");
                break;
            }
        }

        // If all triangles were clipped out, return 0
        if (*num_next == 0) {
            return 0;
        }

        // Swap the buffer pointers for the next iteration
        brh_triangle* temp_ptr = current;
        current = next;
        next = temp_ptr;

        // Swap the count pointers
        int* temp_count = num_current;
        num_current = num_next;
        num_next = temp_count;
    }

    // Copy the final clipped triangles to the output
    int output_count = (*num_current <= MAX_CLIPPED_TRIANGLES)
        ? *num_current : MAX_CLIPPED_TRIANGLES;

    for (int i = 0; i < output_count; i++) {
        output_triangles[i] = current[i];
    }

    return output_count;
}


void clip_polygon(brh_polygon* polygon)
{
	clip_polygon_against_frustum_plane(polygon, FRUSTUM_LEFT);
	clip_polygon_against_frustum_plane(polygon, FRUSTUM_RIGHT);
	clip_polygon_against_frustum_plane(polygon, FRUSTUM_BOTTOM);
	clip_polygon_against_frustum_plane(polygon, FRUSTUM_TOP);
	clip_polygon_against_frustum_plane(polygon, FRUSTUM_NEAR);
	clip_polygon_against_frustum_plane(polygon, FRUSTUM_FAR);
}