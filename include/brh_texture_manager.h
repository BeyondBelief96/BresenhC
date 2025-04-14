#pragma once

#include <stdbool.h>
#include <stdint.h>

// Opaque handle to a texture (hides implementation details)
typedef struct brh_texture_handle_t* brh_texture_handle;

/**
 * @brief Initialize the texture management system
 *
 * @return true if initialization succeeded, false otherwise
 */
bool initialize_texture_system(void);

void cleanup_texture_system(void);

/**
 * @brief Initialize the texture management system
 *
 * @return true if initialization succeeded, false otherwise
 */
bool initialize_texture_system(void);

/**
 * @brief Clean up the texture management system and free all resources
 */
void cleanup_texture_system(void);

/**
 * @brief Load a texture from a PNG file
 *
 * @param file_path Path to the PNG file
 * @return A handle to the loaded texture, or NULL if loading failed
 */
brh_texture_handle load_texture(const char* file_path);

/**
 * @brief Unload a texture and free its resources
 *
 * @param texture_handle Handle to the texture to unload
 */
void unload_texture(brh_texture_handle texture_handle);

/**
 * @brief Get the texture data for rendering
 *
 * @param texture_handle Handle to the texture
 * @return Pointer to the texture data, or NULL if invalid handle
 */
uint32_t* get_texture_data(brh_texture_handle texture_handle);

/**
 * @brief Get the width of a texture
 *
 * @param texture_handle Handle to the texture
 * @return The width of the texture, or 0 if invalid handle
 */
int get_texture_width(brh_texture_handle texture_handle);

/**
 * @brief Get the height of a texture
 *
 * @param texture_handle Handle to the texture
 * @return The height of the texture, or 0 if invalid handle
 */
int get_texture_height(brh_texture_handle texture_handle);