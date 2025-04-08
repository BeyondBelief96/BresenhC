#pragma once

#include <stdint.h>
#include "brh_vector.h"
#include "brh_texture.h"


/**
 * @struct brh_face
 * @brief Represents a face of a triangle mesh using vertex and texture coord indices.
 *
 * Stores 1-based indices into the mesh's vertices and texcoords arrays.
 * Also stores a color (e.g., for flat shading or material).
 *
 * @var brh_face::a, brh_face::b, brh_face::c
 * 1-based indices of the vertex positions in the mesh's `vertices` array.
 * 
 * @var brh_face::a_vt, brh_face::b_vt, brh_face::c_vt
 * 1-based indices of the texture coordinates in the mesh's `texcoords` array.
 * A value of 0 indicates no texture coordinate was specified for that vertex in the OBJ.
 * 
 * @var brh_face::a_vn, brh_face::b_vn, brh_face::c_vn
 * 1-based indices of the vertex normals in the mesh's `normals` array.
 * 
 * @var brh_face::color
 * Color associated with the face (e.g., from material or default).
 */
typedef struct {
  int a;
  int b;
  int c;
  int a_vt; 
  int b_vt; 
  int c_vt; 
  int a_vn;
  int b_vn;
  int c_vn;
  uint32_t color;
} brh_face;

/*
* @brief Given three points representing a face, calculates the normal of the face.
* 
* This function calculates the normal of a face given three points that represent the face. The normal is calculated assuming a left-handed
* 
* @param a The first point of the face.
* @param b The second point of the face.
* @param c The third point of the face.
* 
* @return The normal of the face.
*/
brh_vector3 get_face_normal(brh_vector3 a, brh_vector3 b, brh_vector3 c);