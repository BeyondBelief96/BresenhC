#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "math_utils.h"
#include "display.h"
#include "brh_vector3.h"
#include "brh_vector2.h"
#include "brh_mesh.h"
#include "brh_triangle.h"
#include "array.h"
#include "model_loader.h"

brh_triangle* triangles_to_render = NULL;

bool is_running = true;
uint32_t cell_size;

float fov_factor = 640;
	
brh_vector3 camera_position = { .x = 0, .y = 0, .z = 0 };

brh_vector2 project(brh_vector3 point)
{
	brh_vector2 projected_point = {
		.x = (point.x  / point.z) * fov_factor,
		.y = (point.y / point.z) * fov_factor ,
	};

	return projected_point;
}

bool should_cull_face_manual(brh_vector3* vertices, brh_vector3 camera_position)
{
	// Check for backface culling
	/*
	*     A
	*	/   \
	*  C-----B
	*/
	brh_vector3 vecA = vertices[0];
	brh_vector3 vecB = vertices[1];
	brh_vector3 vecC = vertices[2];
	brh_vector3 vecAB = vec3_subtract(vecB, vecA);
	brh_vector3 vecAC = vec3_subtract(vecC, vecA);
	brh_vector3 face_normal = vec3_cross(vecAB, vecAC);
	vec3_normalize(&face_normal);
	// Goes from vector a on the face to the camera position
	brh_vector3 view_vector = vec3_subtract(camera_position, vecA);
	float angle_between = vec3_dot(face_normal, view_vector);
	if (angle_between < 0)
	{
		return true;
	}
	return false;
}

bool should_cull_face(brh_vector3 face_normal, brh_vector3 camera_position)
{
	brh_vector3 view_vector = vec3_subtract(camera_position, face_normal);
	float angle_between = vec3_dot(face_normal, view_vector);
	return angle_between < 0;
}

void setup(void)
{
	// Allocate back buffera
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
	assert(color_buffer != NULL); // Ensure malloc succeeded
	if (!color_buffer)
	{
		assert(color_buffer);
		fprintf(stderr, "Color buffer could not be created!");
		return;
	}

	// Create color buffer texture
	color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
	if (!color_buffer_texture)
	{
		fprintf(stderr, "Color buffer texture could not be created! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	cell_size = gcd(window_width, window_height);

	//bool loaded = load_gltf("./assets/supermarine_spitfire/scene.gltf", &mesh);
	bool loaded = load_obj("./assets/f22.obj", &mesh);
	if (!loaded)
	{
		fprintf(stderr, "Error loading OBJ file\n");
		return;
	}
}

void process_input(void)
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
		case SDL_EVENT_QUIT:
			is_running = false;
			break;
		case SDL_EVENT_KEY_DOWN:
			if (event.key.key == SDLK_ESCAPE)
			{
				is_running = false;
			}
			break;
	}
}

void update(void)
{
	mesh.rotation.x += 0.001f;
	mesh.rotation.y += 0.001f;
	mesh.rotation.z += 0.001f;

	// Initialize the array of triangles to render
	triangles_to_render = NULL;

	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++)
	{
		brh_face face = mesh.faces[i];
		brh_vector3 face_vertices[3];
		face_vertices[0] = mesh.vertices[face.a - 1];
		face_vertices[1] = mesh.vertices[face.b - 1];
		face_vertices[2] = mesh.vertices[face.c - 1];

		brh_vector3 transformed_vertices[3];
		brh_triangle projected_triangle;
		for (int j = 0; j < 3; j++)
		{
			brh_vector3 vertex = face_vertices[j];

			// Preform rotations
			brh_vector3 transformed_vertex = vec3_rotate_x(vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			// Translate the vertex away from the camera by the camera position
			transformed_vertex.z += 5;
			transformed_vertices[j] = transformed_vertex;
		}

		if (should_cull_face_manual(transformed_vertices, camera_position))
		{
			continue;
		}

		for (int j = 0; j < 3; j++)
		{
			// Project the vertex from 3D World space to 2D screen space
			brh_vector2 projected_point = project(transformed_vertices[j]);

			// Scale and translate the projected point to the center of the screen
			projected_point.x += window_width / 2;
			projected_point.y += window_height / 2;
			projected_triangle.points[j] = projected_point;
		}

		// Save each projected triangle for each face
		array_push(triangles_to_render, projected_triangle);
	}
}

void render(void)
{
	clear_color_buffer(0xFF000000);
	draw_grid(cell_size, 0xFF333333);

	int num_triangles = array_length(triangles_to_render);

	for (int i = 0; i < num_triangles; i++)
	{
		brh_triangle triangle = triangles_to_render[i];
		draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFF00FF00);
		draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFF00FF00);
		draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFF00FF00);

		draw_triangle(triangle, 0xFF00FF00);
	}

	// Clear the array of triangles to render every frame loop
	array_free(triangles_to_render);

	render_color_buffer();

	SDL_RenderPresent(renderer);
}

void free_resources(void)
{
	free(color_buffer);
	array_free(mesh.vertices);
	array_free(mesh.faces);
}

int main(int argc, char* argv[])
{
	is_running = initialize_window();
	setup();

	while (is_running)
	{
		process_input();
		update();
		render();
	}

	destroy_window();
	free_resources();

	return 0;
}