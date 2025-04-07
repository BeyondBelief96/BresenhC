#include <stdlib.h>
#include "brh_mesh.h"
#include "brh_face.h"
#include "array.h"

/*
		7---------5
	   /|        /|
	  / |       / |
	 2---------3  |
	 |  |      |  |
	 |  8------|--6
	 | /       | /
	 |/        |/
	 1---------4

Vertices:
1: { .x = -1, .y = -1, .z = -1 }
2: { .x =  1, .y = -1, .z = -1 }
3: { .x =  1, .y =  1, .z = -1 }
4: { .x = -1, .y =  1, .z = -1 }
5: { .x = -1, .y = -1, .z =  1 }
6: { .x =  1, .y = -1, .z =  1 }
7: { .x =  1, .y =  1, .z =  1 }
8: { .x = -1, .y =  1, .z =  1 }
*/

brh_mesh mesh = {
	.vertices = NULL,
	.faces = NULL,
	.rotation = {.x = 0.0f, .y = 0.0f, .z = 0.0f },
	.scale = {.x = 1.0f, .y = 1.0f, .z = 1.0f },
	.translation = {.x = 0.0f, .y = 0.0f, .z = 0.0f }
};


brh_vector3 cube_vertices[N_CUBE_VERTICES] = {
	{.x = -1, .y = -1, .z = -1 },  // Vertex 1: front-bottom-left
	{.x = -1, .y = 1, .z = -1 },   // Vertex 2: front-top-left
	{.x = 1, .y = 1, .z = -1 },    // Vertex 3: front-top-right
	{.x = 1, .y = -1, .z = -1 },   // Vertex 4: front-bottom-right
	{.x = 1, .y = 1, .z = 1 },     // Vertex 5: back-top-right
	{.x = 1, .y = -1, .z = 1 },    // Vertex 6: back-bottom-right
	{.x = -1, .y = 1, .z = 1 },    // Vertex 7: back-top-left
	{.x = -1, .y = -1, .z = 1 },   // Vertex 8: back-bottom-left
};

brh_face cube_faces[N_CUBE_FACES] = {
	// front
	{.a = 1, .b = 2, .c = 3, .texel_a = { 0, 1 }, .texel_b = { 0, 0 }, .texel_c = { 1, 0 }, .color = 0xFFFFFFFF },
	{.a = 1, .b = 3, .c = 4, .texel_a = { 0, 1 }, .texel_b = { 1, 0 }, .texel_c = { 1, 1 }, .color = 0xFFFFFFFF },
	// right
	{.a = 4, .b = 3, .c = 5, .texel_a = { 0, 1 }, .texel_b = { 0, 0 }, .texel_c = { 1, 0 }, .color = 0xFFFFFFFF },
	{.a = 4, .b = 5, .c = 6, .texel_a = { 0, 1 }, .texel_b = { 1, 0 }, .texel_c = { 1, 1 }, .color = 0xFFFFFFFF },
	// back
	{.a = 6, .b = 5, .c = 7, .texel_a = { 0, 1 }, .texel_b = { 0, 0 }, .texel_c = { 1, 0 }, .color = 0xFFFFFFFF },
	{.a = 6, .b = 7, .c = 8, .texel_a = { 0, 1 }, .texel_b = { 1, 0 }, .texel_c = { 1, 1 }, .color = 0xFFFFFFFF },
	// left
	{.a = 8, .b = 7, .c = 2, .texel_a = { 0, 1 }, .texel_b = { 0, 0 }, .texel_c = { 1, 0 }, .color = 0xFFFFFFFF },
	{.a = 8, .b = 2, .c = 1, .texel_a = { 0, 1 }, .texel_b = { 1, 0 }, .texel_c = { 1, 1 }, .color = 0xFFFFFFFF },
	// top
	{.a = 2, .b = 7, .c = 5, .texel_a = { 0, 1 }, .texel_b = { 0, 0 }, .texel_c = { 1, 0 }, .color = 0xFFFFFFFF },
	{.a = 2, .b = 5, .c = 3, .texel_a = { 0, 1 }, .texel_b = { 1, 0 }, .texel_c = { 1, 1 }, .color = 0xFFFFFFFF },
	// bottom
	{.a = 6, .b = 8, .c = 1, .texel_a = { 0, 1 }, .texel_b = { 0, 0 }, .texel_c = { 1, 0 }, .color = 0xFFFFFFFF },
	{.a = 6, .b = 1, .c = 4, .texel_a = { 0, 1 }, .texel_b = { 1, 0 }, .texel_c = { 1, 1 }, .color = 0xFFFFFFFF }
};

void load_cube_mesh(void)
{
	for (int i = 0; i < N_CUBE_VERTICES; i++)
	{
		array_push(mesh.vertices, cube_vertices[i]);
	}

	for (int i = 0; i < N_CUBE_FACES; i++)
	{
		array_push(mesh.faces, cube_faces[i]);
	}

	mesh.rotation = (brh_vector3){ .x = 0, .y = 0, .z = 0 };
}