#pragma once

#include <stdint.h>
#include "brh_vector.h"
#include "brh_texture.h"

/**
 * @struct brh_face
 * @brief Represents a face of a triangle mesh using vertex indexes.
 *
 * This structure defines a face in a triangle mesh by storing the indexes
 * of its three vertices. Each index corresponds to a vertex in the mesh's
 * vertex array.
 *
 * @var brh_face::a
 * Index of the first vertex of the face.
 *
 * @var brh_face::b
 * Index of the second vertex of the face.
 *
 * @var brh_face::c
 * Index of the third vertex of the face.
 * 
 * @var brh_face::texel_a
 * Texture coordinates (u,v) for the first vertex of the face. This is used for texture mapping.
 * 
 * @var brh_face::texel_b
 * Texture coordinates (u,v) for the second vertex of the face. This is used for texture mapping.
 * 
 * @var brh_face::texel_c
 * Texture coordinates (u,v) for the third vertex of the face. This is used for texture mapping.
 */
typedef struct {
  int a;
  int b;
  int c;
  brh_texel texel_a; 
  brh_texel texel_b; 
  brh_texel texel_c; 
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