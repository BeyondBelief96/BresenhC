#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "obj_loader.h"
#include "array.h"
#include "mesh.h"

/*
* Loads an OBJ file into a mesh struct and returns true if successful
* 
* @param file_path The path to the OBJ file
* @param mesh The mesh struct to load the OBJ file into
* 
* @return true if the OBJ file was loaded successfully, false otherwise
*/
bool load_obj(const char* file_path, brh_mesh* mesh)
{
	FILE* file;
	file = fopen(file_path, "r");
	if (file == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n", file_path);
		return false;
	}

	char line[1024];

	while (fgets(line, 1024, file) != NULL)
	{
		// For now though, I want to ignore the normals
		// and texture coordinates that begin with 'vn' and 'vt'
		if (strncmp(line, "v ", 2) == 0)
		{
			brh_vector3 vertex;
			errno_t err = sscanf(line, "v %f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			if (err != 3)
			{
				fprintf(stderr, "Error parsing vertex data\n");
				return false;
			}
			array_push(mesh->vertices, vertex);
		}
		else if (line[0] == 'f')
		{
			brh_face face;
			int vertex_index[3];
			// face data is formatted like f 1/1/1 2/2/1 3/3/1, where the first number is the vertex index
			// and the second number is the texture index and the third number is the normal index
			errno_t err = sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d\n", &face.a, &face.b, &face.c);
			if (err != 3)
			{
				fprintf(stderr, "Error parsing face data\n");
				return false;
			}
			array_push(mesh->faces, face);
		}
	}
}