#pragma once

#include <stdbool.h>
#include "vector.h"
#include "mesh.h"

bool load_obj(const char* file_path, brh_mesh* mesh);
