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