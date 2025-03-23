#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;

extern uint32_t window_width;
extern uint32_t window_height;


bool initialize_window(void);
extern void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_grid(int cell_size, uint32_t color);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void destroy_window(void);
