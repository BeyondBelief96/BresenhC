#pragma once

#include <stdbool.h>
#include "brh_mesh.h"

bool load_obj(const char* file_path, brh_mesh* mesh, bool isRightHanded);
bool load_gltf(const char* file_path, brh_mesh* mesh);
