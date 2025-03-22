#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "display.h"
#include "vector.h"
#include "math.h"

#define N_POINTS 9 * 9 * 9

bool is_running = true;
uint32_t cell_size;

float fov_factor = 640;

brh_vector3 cube_points[N_POINTS];
brh_vector2 projected_points[N_POINTS];
	
brh_vector3 camera_position = { .x = 0, .y = 0, .z = -5 };
brh_vector3 cube_rotation = { .x = 0, .y = 0, .z = 0 };

void generate_cube_points(float cube_side_length)
{
	int point_count = 0;
	float step = 2.0f / (cube_side_length - 1); 

	for (float x = -1.0f; x <= 1.0f + 0.0001f; x += step)
	{
		for (float y = -1.0f; y <= 1.0f + 0.0001f; y += step)
		{
			for (float z = -1.0f; z <= 1.0f + 0.0001f; z += step)
			{
				if (point_count < N_POINTS)
				{
					brh_vector3 point = { .x = x, .y = y, .z = z };
					cube_points[point_count++] = point;
				}
			}
		}
	}
}

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
	
	generate_cube_points(9);
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
	cube_rotation.y += 0.05f;
	cube_rotation.z += 0.01f;
	cube_rotation.x += 0.01f;

	for (int i = 0; i < N_POINTS; i++)
	{
		// Grab the original point
		brh_vector3 point = cube_points[i];

		// Rotate the point around the y-axis
		brh_vector3 transformed_point = vec3_rotate_y(point, cube_rotation.y);
		transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);
		transformed_point = vec3_rotate_x(transformed_point, cube_rotation.x);

		// Translate the point away from the camera
		transformed_point.z -= camera_position.z;

		// Project the transformed point onto the 2D screen
		brh_vector2 projected_point = project(transformed_point);
		projected_points[i] = projected_point;
	}
}

void render(void)
{
	clear_color_buffer(0xFF000000);
	draw_grid(cell_size, 0xFF333333);

	for (int i = 0; i < N_POINTS; i++)
	{
		brh_vector2 projected_point = projected_points[i];
		draw_rect(projected_point.x + window_width / 2.0f, projected_point.y + window_height / 2.0f, 5, 5, 0xFF00FF00);
	}

	render_color_buffer();

	SDL_RenderPresent(renderer);
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

	return 0;
}