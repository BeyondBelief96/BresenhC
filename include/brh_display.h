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
};

enum render_method {
    RENDER_WIREFRAME,
    RENDER_WIREFRAME_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIREFRAME,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIREFRAME,
};

/*
* @brief Gets the width of the SDL window.
*
* This function retrieves the current width of the SDL window.
*
* @return The width of the window in pixels.
*/
int get_window_width(void);

/*
* @brief Gets the height of the SDL window.
*
* This function retrieves the current height of the SDL window.
*
* @return The height of the window in pixels.
*/
int get_window_height(void);

/*
* @brief Gets the aspect ratio of the SDL window.
* 
* This function calculates the aspect ratio of the SDL window based on its width and height.
* 
* @return The aspect ratio of the window (width / height).
*/
float get_aspect_ratio(void);

/*
* @brief Sets the size of the SDL window.
*
* This function sets the size of the SDL window to the specified width and height.
*
* @param width The desired width of the window in pixels.
* @param height The desired height of the window in pixels.
*
* @return void
*/
void set_window_size(int width, int height);

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
 * @brief Initializes all display resources (window, buffers, textures).
 *
 * This function initializes the SDL window, renderer, color buffer, z-buffer,
 * and color buffer texture.
 *
 * @return true if all resources were successfully initialized, false otherwise.
 */
bool initialize_display_resources(void);

/**
 * @brief Gets the SDL window.
 *
 * This function retrieves the SDL window object.
 *
 * @return A pointer to the SDL window.
 */
SDL_Window* get_window(void);

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
 * @brief Cleans up all display resources.
 *
 * This function frees memory and releases resources associated with the display
 * (window, renderer, buffers, textures).
 *
 * @return void
 */
void cleanup_display_resources(void);

/**
 * @brief Gets the SDL renderer.
 *
 * This function retrieves the SDL renderer object.
 *
 * @return A pointer to the SDL renderer.
 */
SDL_Renderer* get_renderer(void);

/**
 * @brief Gets the color buffer.
 *
 * This function retrieves the color buffer array.
 *
 * @return A pointer to the color buffer.
 */
uint32_t get_color_buffer_at(int x, int y);

/**
 * @brief Sets the color buffer at the specified coordinates.
 *
 * This function sets the color of a pixel in the color buffer at the specified
 * coordinates.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param color The color to set (in ARGB format).
 *
 * @return void
 */
void set_color_buffer_at(int x, int y, uint32_t color);

/**
 * @brief Gets the z-buffer.
 *
 * This function retrieves the z-buffer array.
 *
 * @return A pointer to the z-buffer.
 */
float get_z_buffer_at(int x, int y);

/**
 * @brief Sets the z-buffer at the specified coordinates.
 *
 * This function sets the depth value of a pixel in the z-buffer at the specified
 * coordinates.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param depth The depth value to set (higher means closer).
 *
 * @return void
 */
void set_z_buffer_at(int x, int y, float depth);

/**
 * @brief Gets the color buffer texture.
 *
 * This function retrieves the SDL texture used for rendering the color buffer.
 *
 * @return A pointer to the color buffer texture.
 */
SDL_Texture* get_color_buffer_texture(void);

/*
* @brief Gets the current rendering method.
*
* This function retrieves the current rendering method used for drawing graphics.
*
* @return The current rendering method.
*/
enum render_method get_render_method(void);

/*
* @brief Gets the current culling method.
*
* This function retrieves the current culling method used for rendering triangles.
*
* @return The current culling method.
*/
enum cull_method get_cull_method(void);

/**
 * @brief Sets the rendering method.
 *
 * This function sets the rendering method to be used for drawing graphics.
 *
 * @param method The rendering method to set.
 *
 * @return void
 */
void set_render_method(enum render_method method);

/**
 * @brief Sets the culling method.
 *
 * This function sets the culling method to be used for rendering triangles.
 *
 * @param method The culling method to set.
 *
 * @return void
 */
void set_cull_method(enum cull_method method);

/**
 * @brief Draws a pixel on the screen at the specified coordinates with the specified color.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param color The color of the pixel.
 *
 * @return void
 */
void draw_pixel(int x, int y, uint32_t color);

/**
 * @brief Draws a pixel with depth checking.
 *
 * This function draws a pixel only if it's closer to the camera than any previously
 * drawn pixel at the same coordinates.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param depth The depth value (higher means closer).
 * @param color The color of the pixel.
 *
 * @return void
 */
void draw_pixel_with_depth(int x, int y, float depth, uint32_t color);

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