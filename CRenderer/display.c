#include <stdio.h>
#include <stdlib.h>
#include "display.h"


SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;

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
	free(color_buffer);
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