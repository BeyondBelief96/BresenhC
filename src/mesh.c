#include <stdlib.h>
#include "mesh.h"
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
	.rotation = {.x = 0, .y = 0, .z = 0 },
};


brh_vector3 cube_vertices[N_CUBE_VERTICES] = {
	{.x = -1, .y = -1, .z = -1 },  // Vertex 1: front-bottom-left
	{.x = -1, .y = -1, .z = 1 },   // Vertex 2: front-top-left
	{.x = 1, .y = -1, .z = 1 },    // Vertex 3: front-top-right
	{.x = 1, .y = -1, .z = -1 },   // Vertex 4: front-bottom-right
	{.x = 1, .y = 1, .z = 1 },     // Vertex 5: back-top-right
	{.x = 1, .y = 1, .z = -1 },    // Vertex 6: back-bottom-right
	{.x = -1, .y = 1, .z = 1 },    // Vertex 7: back-top-left
	{.x = -1, .y = 1, .z = -1 },   // Vertex 8: back-bottom-left
};

brh_face cube_faces[N_CUBE_FACES] =
{
    // Front
    {.a = 1, .b = 2, .c = 3},
    {.a = 1, .b = 3, .c = 4},
    // Right
    {.a = 4, .b = 3, .c = 5},
    {.a = 4, .b = 5, .c = 6 },
    // Back
    {.a = 6, .b = 5, .c = 7},
    {.a = 6, .b = 7, .c = 8 },
    // Left
    {.a = 8, .b = 7, .c = 2},
    {.a = 8, .b = 2, .c = 1 },
    // Top
    {.a = 2, .b = 7, .c = 5},
    {.a = 2, .b = 5, .c = 3 },
    // Bottom
    {.a = 6, .b = 8, .c = 1 },
    {.a = 6, .b = 1, .c = 4 },
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