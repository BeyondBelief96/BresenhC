#pragma once

#include <stdbool.h>
#include "brh_mesh.h"

/**
 * @brief Loads vertex positions, texture coordinates, and face indices from an OBJ file.
 *
 * Handles common face formats: v/vt/vn, v/vt, v//vn, v.
 * Populates the mesh struct's vertices, texcoords, and faces arrays.
 * Converts coordinates to left-handed if isRightHanded is true.
 *
 * @param file_path Path to the OBJ file.
 * @param mesh Pointer to the mesh structure to populate.
 * @param isRightHanded If true, converts Z-coordinates and reverses face winding order.
 * @return true if loading was successful, false otherwise.
 */
bool load_obj(const char* file_path, brh_mesh* mesh, bool isRightHanded);

/*
* Loads a glTF file into a mesh struct and returns true if successful
*
* @param file_path The path to the glTF file
* @param mesh The mesh struct to load the glTF file into
*
* @return true if the glTF file was loaded successfully, false otherwise
*/
bool load_gltf(const char* file_path, brh_mesh* mesh);
