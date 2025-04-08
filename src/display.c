#include <stdio.h>
#include <stdlib.h>
#include "math_utils.h"
#include "display.h"

enum cull_method cull_method = CULL_NONE;
enum render_method render_method = RENDER_WIREFRAME;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;
float* z_buffer = NULL;

uint32_t window_width = 800;
uint32_t window_height = 600;

bool initialize_window()
{
	if (SDL_Init(SDL_INIT_VIDEO) == false)
	{
		fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Use SDL to query what is the fullscreeen size of the display
	SDL_DisplayID display_id = SDL_GetPrimaryDisplay();
	const SDL_DisplayMode* display_mode = SDL_GetCurrentDisplayMode(display_id);

	window_width = display_mode->w;
	window_height = display_mode->h;

	window = SDL_CreateWindow("C Renderer", window_width, window_height, 0);

	if (!window)
	{
		fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer)
	{
		fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetWindowFullscreen(window, true);

	return true;
}

void destroy_window(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void clear_color_buffer(uint32_t color)
{
	for (uint32_t y = 0; y < window_height; y++)
	{
		for (uint32_t x = 0; x < window_width; x++)
		{
			color_buffer[(window_width * y) + x] = color;
		}
	}
}

/**
 * @brief Clears the Z-buffer, setting all depth values to represent maximum distance.
 *
 * This function resets the Z-buffer, typically before rendering a new frame.
 * We initialize with 0.0f because we will store 1/w, and larger 1/w means closer.
 */
void clear_z_buffer(void)
{
	if (!z_buffer) return; // Safety check
	const int num_pixels = window_width * window_height;
	for (int i = 0; i < num_pixels; i++)
	{
		z_buffer[i] = 0.0f; // Initialize to "infinitely far" (smallest possible 1/w)
		// Using FLT_MIN might also work, but 0.0 is common.
	}
}

void render_color_buffer(void)
{
	SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)(window_width * sizeof(uint32_t)));
	SDL_RenderTexture(renderer, color_buffer_texture, NULL, NULL);
}

inline void draw_pixel(uint32_t x, uint32_t y, uint32_t color)
{
	if (x >= 0 && x < window_width && y >= 0 && y < window_height)
	{
		color_buffer[(window_width * y) + x] = color;
	}
}

void draw_grid(int cell_size, uint32_t color)
{
	// Draw vertical lines
	for (uint32_t x = 0; x < window_width; x += (uint32_t)cell_size)
	{
		for (int y = 0; y < (int)window_height; y++)
		{
			draw_pixel(x, y, color);
		}
	}

	// Draw horizontal lines
	for (uint32_t y = 0; y < window_height; y += cell_size)
	{
		for (uint32_t x = 0; x < window_width; x++)
		{
			draw_pixel(x, y, color);
		}
	}
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
	for (int i = x; i < x + width; i++)
	{
		for (int j = y; j < y + height; j++)
		{
			draw_pixel(i, j, color);
		}
	}
}

void draw_line_dda(int x0, int y0, int x1, int y1, uint32_t color)
{
	int delta_x = x1 - x0;
	int delta_y = y1 - y0;

	// Calculate which axis has the greater distance
	int steps = MAX(abs(delta_x), abs(delta_y));

	// Calculate increment in x and y for each step, one of these will be 1, while the other will be the slope.
	float increment_x = (float)delta_x / (float)steps;
	float increment_y = (float)delta_y / (float)steps;

	// Casting to float to avoid truncation since inrement is a float, intial values are x0 and y0.
	float current_x = (float)x0;
	float current_y = (float)y0;
	for (int i = 0; i <= steps; i++)
	{
		draw_pixel((uint32_t)current_x, (uint32_t)current_y, color);
		current_x += increment_x;
		current_y += increment_y;
	}
}

void draw_horizontal_line(int x0, int x1, int y, uint32_t color)
{
	if (x0 > x1)
	{
		swap_int(&x0, &x1);
	}

	for (int x = x0; x <= x1; x++)
	{
		draw_pixel(x, y, color);
	}
}