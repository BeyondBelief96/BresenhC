#include <stdio.h>
#include <stdlib.h>
#include "math_utils.h"
#include "brh_display.h"

static enum cull_method cull_method = CULL_NONE;
static enum render_method render_method = RENDER_WIREFRAME;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

static uint32_t* color_buffer = NULL;
static SDL_Texture* color_buffer_texture = NULL;
static float* z_buffer = NULL;

static int window_width = 800;
static int window_height = 600;

int get_window_width(void)
{
    return window_width;
}

int get_window_height(void)
{
    return window_height;
}

void set_window_size(int width, int height)
{
    window_width = width;
    window_height = height;
    if (window)
    {
        SDL_SetWindowSize(window, window_width, window_height);
    }
    else
    {
        fprintf(stderr, "Error: Window not initialized\n");
    }
}

bool initialize_window(void)
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
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_SetWindowFullscreen(window, true);
    return true;
}

bool initialize_display_resources(void)
{
    // Initialize window and renderer first
    if (!initialize_window())
    {
        return false;
    }

    // Allocate color and z-buffers
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
    z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

    if (!color_buffer || !z_buffer)
    {
        fprintf(stderr, "Error: Failed to allocate color or Z buffer\n");
        cleanup_display_resources();
        return false;
    }

    // Create color buffer texture
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    if (!color_buffer_texture)
    {
        fprintf(stderr, "Error: Failed to create color buffer texture: %s\n", SDL_GetError());
        cleanup_display_resources();
        return false;
    }

    // Initialize buffers
    clear_color_buffer(0xFF000000);  // Black
    clear_z_buffer();

    return true;
}

SDL_Window* get_window(void)
{
    return window;
}

void destroy_window(void)
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();
}

void cleanup_display_resources(void)
{
    if (color_buffer_texture)
    {
        SDL_DestroyTexture(color_buffer_texture);
        color_buffer_texture = NULL;
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    if (color_buffer)
    {
        free(color_buffer);
        color_buffer = NULL;
    }

    if (z_buffer)
    {
        free(z_buffer);
        z_buffer = NULL;
    }

    SDL_Quit();
}

SDL_Renderer* get_renderer(void)
{
    return renderer;
}

uint32_t* get_color_buffer(void)
{
    return color_buffer;
}

float* get_z_buffer(void)
{
    return z_buffer;
}

SDL_Texture* get_color_buffer_texture(void)
{
    return color_buffer_texture;
}

enum render_method get_render_method(void)
{
    return render_method;
}

enum cull_method get_cull_method(void)
{
    return cull_method;
}

void clear_color_buffer(uint32_t color)
{
    if (!color_buffer) return;

    for (int i = 0; i < window_width * window_height; i++)
    {
        color_buffer[i] = color;
    }
}

void clear_z_buffer(void)
{
    if (!z_buffer) return;

    const int num_pixels = window_width * window_height;
    for (int i = 0; i < num_pixels; i++)
    {
        z_buffer[i] = 0.0f; // Initialize to "infinitely far" (smallest possible 1/w)
    }
}

void render_color_buffer(void)
{
    if (!color_buffer || !color_buffer_texture || !renderer) return;

    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, window_width * sizeof(uint32_t));
    SDL_RenderTexture(renderer, color_buffer_texture, NULL, NULL);
}

void set_render_method(enum render_method method)
{
    render_method = method;
}

void set_cull_method(enum cull_method method)
{
    cull_method = method;
}

void draw_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= window_width || y < 0 || y >= window_height || !color_buffer)
    {
        return;
    }

    color_buffer[(window_width * y) + x] = color;
}

void draw_pixel_with_depth(int x, int y, float depth, uint32_t color)
{
    if (x < 0 || x >= window_width || y < 0 || y >= window_height || !color_buffer || !z_buffer)
    {
        return;
    }

    int index = (window_width * y) + x;
    if (depth > z_buffer[index])
    {
        color_buffer[index] = color;
        z_buffer[index] = depth;
    }
}

void draw_grid(int cell_size, uint32_t color)
{
    // Draw vertical lines
    for (int x = 0; x < window_width; x += cell_size)
    {
        for (int y = 0; y < window_height; y++)
        {
            draw_pixel(x, y, color);
        }
    }

    // Draw horizontal lines
    for (int y = 0; y < window_height; y += cell_size)
    {
        for (int x = 0; x < window_width; x++)
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
        draw_pixel((int)current_x, (int)current_y, color);
        current_x += increment_x;
        current_y += increment_y;
    }
}