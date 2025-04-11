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
