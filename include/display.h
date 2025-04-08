#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "brh_triangle.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

enum cull_method {
	CULL_NONE,
	CULL_BACKFACE
} extern cull_method;

enum render_method {
	RENDER_WIREFRAME,
	RENDER_WIREFRAME_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIREFRAME,
	RENDER_TEXTURED,
	RENDER_TEXTURED_WIREFRAME,
} extern render_method;

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;
extern float* z_buffer;

extern uint32_t window_width;
extern uint32_t window_height;


/**
 * @brief Initializes the SDL window and renderer.
 *
 * This function initializes the SDL window and renderer, setting up the necessary
 * resources for rendering graphics.
 *
 * @return true if the window and renderer were successfully initialized, false otherwise.
 */
bool initialize_window(void);

/**
 * @brief Destroys the SDL window and renderer.
 *
 * This function cleans up and releases the resources associated with the SDL window
 * and renderer.
 *
 * @return void
 */
void destroy_window(void);

/**
 * @brief Draws a pixel on the screen at the specified coordinates with the specified color.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param color The color of the pixel.
 *
 * @return void
 */
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);

/**
 * @brief Draws a rectangle on the screen at the specified coordinates with the specified color.
 *
 * @param x The x coordinate of the top-left corner of the rectangle.
 * @param y The y coordinate of the top-left corner of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param color The color of the rectangle.
 *
 * @return void
 */
void draw_rect(int x, int y, int width, int height, uint32_t color);

/**
 * @brief Draws a line using the DDA algorithm.
 *
 * This function draws a line using the Digital Differential Analyzer (DDA) algorithm.
 *
 * @param x0 The x coordinate of the starting point.
 * @param y0 The y coordinate of the starting point.
 * @param x1 The x coordinate of the ending point.
 * @param y1 The y coordinate of the ending point.
 * @param color The color of the line.
 *
 * @return void
 */
void draw_line_dda(int x0, int y0, int x1, int y1, uint32_t color);

/**
 * @brief Draws a grid on the screen.
 *
 * This function draws a grid on the screen with the specified cell size and color.
 *
 * @param cell_size The size of each cell in the grid.
 * @param color The color of the grid lines.
 *
 * @return void
 */
void draw_grid(int cell_size, uint32_t color);

/**
 * @brief Renders the color buffer to the screen.
 *
 * This function updates the SDL texture with the contents of the color buffer and
 * renders it to the screen.
 *
 * @return void
 */
void render_color_buffer(void);

/**
 * @brief Clears the color buffer with the specified color.
 *
 * This function fills the color buffer with the specified color, effectively clearing
 * the screen.
 *
 * @param color The color to fill the buffer with.
 *
 * @return void
 */
void clear_color_buffer(uint32_t color);

/*
* @brief Clears the Z-buffer.
 *
 * This function resets the Z-buffer to its initial state, preparing it for the next
 * frame of rendering.
 *
 * @return void
*/
void clear_z_buffer(void);