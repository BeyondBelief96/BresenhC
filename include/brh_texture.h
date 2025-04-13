#pragma once

#include <stdbool.h>
#include "upng.h"

typedef struct brh_texel{
  float u;
  float v;
} brh_texel;


extern int texture_width;
extern int texture_height;
extern uint32_t* mesh_texture_data;

extern upng_t* png_texture;

bool load_png_texture_data(const char* file_path);

