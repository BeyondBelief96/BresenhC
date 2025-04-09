#pragma once

#include "brh_vector.h"
#include "brh_triangle.h"
#include "brh_face.h"

/**
 * @struct brh_mesh
 * @brief Represents a 3D mesh composed of vertices, texture coordinates, and faces.
 *
 * Stores arrays for vertex positions, texture coordinates (UVs), and faces.
 * Also includes transformation properties.
 * 
 * @var brh_mesh::vertices
 * Dynamic array of `brh_vector3` structures representing vertex positions.
 * 
 * @var brh_mesh::texcoords
 * Dynamic array of `brh_texel` structures representing texture coordinates (UVs).
 * 
 * @var brh_vector3::normals
 * Dynamic array of `brh_vector3` structures representing vertex normals.
 * 
 * @var brh_mesh::faces
 * Dynamic array of `brh_face` structures defining triangles and linking vertex/texcoord indices.
 * 
 * @var brh_mesh::rotation
 * Mesh rotation (Euler angles).
 * 
 * @var brh_mesh::scale
 * Mesh scale.
 * 
 * @var brh_mesh::translation
 * Mesh translation.
 */
typedef struct {
	brh_vector3* vertices;
	brh_texel* texcoords;
	brh_vector3* normals;
	brh_face* faces;
	brh_vector3 scale;
	brh_vector3 rotation;
	brh_vector3 translation;
} brh_mesh;

extern brh_mesh mesh;