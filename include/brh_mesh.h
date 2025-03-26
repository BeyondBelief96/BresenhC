#pragma once

#include "brh_vector3.h"
#include "brh_triangle.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES 12

extern brh_vector3 cube_vertices[N_CUBE_VERTICES];
extern brh_face cube_faces[N_CUBE_FACES];

/**
 * @struct brh_mesh
 * @brief Represents a 3D mesh composed of vertices and faces.
 *
 * This structure defines a 3D mesh by storing an array of vertices and an array of faces.
 * Each vertex is represented by a `brh_vector3` structure, which contains the x, y, and z
 * coordinates of the point. Each face is represented by a `brh_face` structure, which
 * contains the indexes of the vertices that form the face.
 *
 * @var brh_mesh::vertices
 * Pointer to an array of `brh_vector3` structures representing the vertices of the mesh.
 *
 * @var brh_mesh::faces
 * Pointer to an array of `brh_face` structures representing the faces of the mesh.
 * 
 * @var brh_mesh::rotation
 * Defines the rotation of the mesh in the 3D space using Euler angles.
 */
typedef struct {
	brh_vector3* vertices;
	brh_vector3* normals;
	brh_face* faces;
	brh_vector3 rotation;
} brh_mesh;

extern brh_mesh mesh;

void load_cube_mesh(void);

