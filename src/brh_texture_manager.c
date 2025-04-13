#include <stdlib.h>
#include <stdio.h>
#include "upng.h"
#include "brh_texture_manager.h"

#define MAX_TEXTURES 32  // Maximum number of textures that can be loaded simultaneously

typedef struct brh_texture_data {
	uint32_t* data;			// Texture pixel data
	int width;				// Texture width
	int height;				// Texture height
	upng_t* png;	        // UPNG structure for this texture
} brh_texture_data;

typedef struct brh_texture_handle_t {
	int id;						// Unique identifier for this texture
	brh_texture_data* texture;	// Pointer to the actual texture data
	bool is_valid;				// Whether this handle is valid
} brh_texture_handle_t;

static brh_texture_handle_t texture_handles[MAX_TEXTURES];
static int next_texture_id = 1;  // Start from 1, 0 can be reserved for invalid handles

bool initialize_texture_system(void)
{
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		texture_handles[i].id = 0;
		texture_handles[i].texture = NULL;
		texture_handles[i].is_valid = false;
	}

	next_texture_id = 1;
	return true;
}

void cleanup_texture_system(void)
{
    // Free all valid textures
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (texture_handles[i].is_valid && texture_handles[i].texture != NULL) {
            unload_texture((brh_texture_handle)&texture_handles[i]);
        }
    }
}

brh_texture_handle load_texture(const char* file_path)
{
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (!texture_handles[i].is_valid) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "Error: Maximum number of textures (%d) reached\n", MAX_TEXTURES);
        return NULL;
    }

    // Allocate texture structure
    brh_texture_data* new_texture = (brh_texture_data*)malloc(sizeof(brh_texture_data));
    if (!new_texture) {
        fprintf(stderr, "Error: Failed to allocate memory for texture\n");
        return NULL;
    }

    // Load texture data from file
    upng_t* png = upng_new_from_file(file_path);
    if (png == NULL) {
        fprintf(stderr, "Error: Failed to load texture from file: %s\n", file_path);
        free(new_texture);
        return NULL;
    }

    upng_decode(png);
    if (upng_get_error(png) != UPNG_EOK) {
        fprintf(stderr, "Error: Failed to decode texture: %i\n", upng_get_error_line(png));
        upng_free(png);
        free(new_texture);
        return NULL;
    }

    // Setup texture data
    new_texture->data = (uint32_t*)upng_get_buffer(png);
    new_texture->width = upng_get_width(png);
    new_texture->height = upng_get_height(png);
    new_texture->png = png;

    // Convert RGBA to ARGB format (if needed for your renderer)
    for (int i = 0; i < new_texture->width * new_texture->height; i++) {
        uint32_t color = new_texture->data[i];
        uint32_t a = (color & 0xFF000000);
        uint32_t r = (color & 0x00FF0000) >> 16;
        uint32_t g = (color & 0x0000FF00);
        uint32_t b = (color & 0x000000FF) << 16;
        new_texture->data[i] = (a | r | g | b);
    }

    // Setup the handle
    texture_handles[slot].id = next_texture_id++;
    texture_handles[slot].texture = new_texture;
    texture_handles[slot].is_valid = true;

    return (brh_texture_handle)&texture_handles[slot];
}

void unload_texture(brh_texture_handle texture_handle)
{
    if (!texture_handle || !((brh_texture_handle_t*)texture_handle)->is_valid) {
        return;
    }

    brh_texture_handle_t* handle = (brh_texture_handle_t*)texture_handle;
    brh_texture_data* texture = handle->texture;

    // Free texture resources
    if (texture->png) {
        upng_free(texture->png);
        texture->png = NULL;
        // Note: data is owned by png and will be freed with it
        texture->data = NULL;
    }

    // Free texture structure
    free(texture);

    // Invalidate handle
    handle->texture = NULL;
    handle->is_valid = false;
}

uint32_t* get_texture_data(brh_texture_handle texture_handle)
{
    if (!texture_handle || !((brh_texture_handle_t*)texture_handle)->is_valid) {
        return NULL;
    }

    return ((brh_texture_handle_t*)texture_handle)->texture->data;
}

int get_texture_width(brh_texture_handle texture_handle)
{
    if (!texture_handle || !((brh_texture_handle_t*)texture_handle)->is_valid) {
        return 0;
    }

    return ((brh_texture_handle_t*)texture_handle)->texture->width;
}

int get_texture_height(brh_texture_handle texture_handle)
{
    if (!texture_handle || !((brh_texture_handle_t*)texture_handle)->is_valid) {
        return 0;
    }

    return ((brh_texture_handle_t*)texture_handle)->texture->height;
}