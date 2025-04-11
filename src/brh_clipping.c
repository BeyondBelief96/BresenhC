#include <math.h>
#include <stdio.h>
#include "brh_clipping.h"

/* function declarations*/
static int clip_triangle_against_plane(brh_triangle* triangle, brh_clip_plane plane, brh_triangle* output);

bool is_vertex_inside_clipspace(brh_vector4 v)
{
	return (-v.w <= v.x && v.x <= v.w) &&
		(-v.w <= v.y && v.y <= v.w) &&
		(-v.w <= v.z && v.z <= v.w);
}

bool is_vertex_inside_plane(brh_vector4 v, brh_clip_plane plane) {
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

float get_line_plane_intersection_parameter(brh_vector4 v0, brh_vector4 v1, brh_clip_plane plane) {
    // Calculate intersection parameter t where v0 + t*(v1-v0) is on the plane
    float t = 0.0f;

    switch (plane) 
    {
        case CLIP_LEFT:
            t = (-v0.w - v0.x) / ((v1.x - v0.x) + (v1.w - v0.w));
            break;
        case CLIP_RIGHT:
            t = (v0.w - v0.x) / ((v1.x - v0.x) - (v1.w - v0.w));
            break;
        case CLIP_BOTTOM:
            t = (-v0.w - v0.y) / ((v1.y - v0.y) + (v1.w - v0.w));
            break;
        case CLIP_TOP:
            t = (v0.w - v0.y) / ((v1.y - v0.y) - (v1.w - v0.w));
            break;
        case CLIP_NEAR:
            t = (-v0.w - v0.z) / ((v1.z - v0.z) + (v1.w - v0.w));
            break;
        case CLIP_FAR:
            t = (v0.w - v0.z) / ((v1.z - v0.z) - (v1.w - v0.w));
            break;
        default:
            break;
    }

    return t;
}

/*
* @brief Clips a triangle against a clipping plane.
*
* This function clips a triangle against a specified clipping plane.
* The triangle is defined by its vertices, and the function modifies the
* triangle to fit within the clipping plane. The output triangle is
* returned in the output parameter.
*
* @param triangle The triangle to clip.
* @param plane The clipping plane to clip against.
* @param output The output triangle after clipping.
*
* @return The number of output triangles after clipping.
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

    // Process each edge of the triangle
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

            // Clamp t to avoid precision issues
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            // Generate a new vertex at the intersection point
            new_vertices[num_vertices++] = interpolate_vertices(vertices[i], vertices[j], t);
        }
    }

    // Create output triangles using triangle fan (for convex polygons)
    int num_output_triangles = 0;

    if (num_vertices >= 3) {
        // First triangle
        output[num_output_triangles].vertices[0] = new_vertices[0];
        output[num_output_triangles].vertices[1] = new_vertices[1];
        output[num_output_triangles].vertices[2] = new_vertices[2];
        output[num_output_triangles].color = triangle->color;
        num_output_triangles++;

        // Additional triangles if we have more than 3 vertices
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

    // Clip against each plane
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


