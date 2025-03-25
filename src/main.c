#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "math_utils.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "obj_loader.h"

brh_triangle* triangles_to_render = NULL;

bool is_running = true;
uint32_t cell_size;

float fov_factor = 640;
	
brh_vector3 camera_position = { .x = 0, .y = 0, .z = -5 };

brh_vector2 project(brh_vector3 point)
{
	brh_vector2 projected_point = {
		.x = (point.x  / point.z) * fov_factor,
		.y = (point.y / point.z) * fov_factor ,
	};

	return projected_point;
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

	// Loads our global mesh with the cube vertices and faces
	bool loaded = load_obj("assets/f22.obj", &mesh);
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
	mesh.rotation.y += 0.00f;
	mesh.rotation.z += 0.00f;
	mesh.rotation.x += 0.01f;

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

		brh_triangle projected_triangle;
		for (int j = 0; j < 3; j++)
		{
			brh_vector3 vertex = face_vertices[j];
			brh_vector3 transformed_vertex = vec3_rotate_y(vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);
			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex.z -= camera_position.z;

			brh_vector2 projected_point = project(transformed_vertex);

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