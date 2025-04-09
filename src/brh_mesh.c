#include <stdlib.h>
#include "brh_mesh.h"
#include "brh_face.h"
#include "array.h"

brh_mesh mesh = {
	.vertices = NULL,
	.faces = NULL,
	.rotation = {.x = 0.0f, .y = 0.0f, .z = 0.0f },
	.scale = {.x = 1.0f, .y = 1.0f, .z = 1.0f },
	.translation = {.x = 0.0f, .y = 0.0f, .z = 0.0f }
};