#include <stdio.h>
#include <stdint.h>
#include "brh_texture.h"

int texture_width = 64;
int texture_height = 64;

uint32_t* mesh_texture_data = NULL;
upng_t* png_texture = NULL;

bool load_png_texture_data(const char* file_path)
{
	if (file_path == NULL)
	{
		fprintf(stderr, "Error: file_path is NULL\n");
		return false;
	}

    png_texture = upng_new_from_file(file_path);
	if (png_texture == NULL)
	{
		fprintf(stderr, "Error: Failed to load texture from file %s\n", file_path);
		return false;
	}

	upng_decode(png_texture);
	if (upng_get_error(png_texture) == UPNG_EOK)
	{
        mesh_texture_data = (uint32_t*)upng_get_buffer(png_texture);
        texture_width = upng_get_width(png_texture);
        texture_height = upng_get_height(png_texture);

        for (int i = 0; i < texture_width * texture_height; i++)
        {
            uint32_t color = mesh_texture_data[i];
            uint32_t a = (color & 0xFF000000);
            uint32_t r = (color & 0x00FF0000) >> 16;
            uint32_t g = (color & 0x0000FF00);
            uint32_t b = (color & 0x000000FF) << 16;
            mesh_texture_data[i] = (a | r | g | b);
        }

        return true;
	}
	else
	{
		fprintf(stderr, "Error: Failed to decode texture from file %s\n", file_path);
		upng_free(png_texture);
		return false;
	}
}
