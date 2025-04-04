#pragma once

typedef struct brh_texel{
  float u;
  float v;
} brh_texel;


extern int texture_width;
extern int texture_height;
extern uint32_t* mesh_texture_data;

extern const uint8_t REDBRICK_TEXTURE[];

