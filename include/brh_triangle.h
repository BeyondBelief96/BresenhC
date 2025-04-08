#pragma once

#include <stdint.h>
#include "brh_vector.h" 
#include "brh_texture.h" 

// Structure to hold perspective-correct attributes for perspective-correct texture mapping
typedef struct {
    float u_over_w;
    float v_over_w;
    float inv_w; // 1/w
} brh_perspective_attribs;

/**
 * @struct brh_vertex
 * @brief Represents a vertex with position, texture coordinates, normal, and perspective attribute.
 *
 * Defines a vertex combining its 2D screen-space position (after projection and division),
 * its texture coordinates (u,v) for mapping, its original 3D normal vector,
 * and the inverse of its clip-space W component (1/w) for perspective correction.
 *
 * @var brh_vertex::position
 * A `brh_vector4` storing the final screen-space X and Y. The Z component might store
 * NDC Z (for depth buffering) or original world Z (for painter's algorithm).
 * The W component here is often not directly used after perspective division but might retain the original clip-space W for reference.
 * @var brh_vertex::texel
 * A `brh_texel` representing the (u, v) texture coordinates for the vertex.
 * @var brh_vertex::normal
 * A `brh_vector3` representing the original normal vector (often used pre-projection).
 * @var brh_vertex::inv_w
 * The inverse (1/w) of the vertex's W component in *clip space* (before perspective division).
 * This is crucial for perspective-correct interpolation.
 */
typedef struct {
    brh_vector4 position; // Holds final screen X, Y. Z/W usage depends on depth/sorting method.
    brh_texel texel;
    brh_vector3 normal;
    float inv_w;       // Inverse W (1/w) from clip space
} brh_vertex;

/**
 * @struct brh_triangle
 * @brief Represents a triangle defined by three vertices, color, and depth.
 *
 * Stores the three vertices that define the triangle, each containing position,
 * texture coordinates, and normal. Also includes a color for solid rendering
 * and an average depth for sorting (e.g., Painter's Algorithm).
 *
 * @var brh_triangle::vertices
 * Array of three `brh_vertex` structures defining the triangle's corners.
 * Each vertex holds its position, texel coords, and normal.
 * @var brh_triangle::color
 * A 32-bit color value (e.g., ARGB) for flat shading or wireframe.
 * @var brh_triangle::avg_depth
 * The average depth (z-value, typically pre-projection or 1/w post-projection)
 * used for visibility sorting.
 */
typedef struct {
    brh_vertex vertices[3];
    uint32_t color;
} brh_triangle;

/* Function Prototypes */

/**
 * @brief Draws the wireframe outline of a triangle.
 *
 * Uses the DDA line algorithm (or potentially Bresenham) to draw the three
 * edges connecting the triangle's vertices.
 *
 * @param triangle Pointer to the constant triangle data to draw. Using a pointer
 *                 is more efficient than passing the struct by value.
 * @param color The 32-bit color (e.g., ARGB) for the outline.
 */
void draw_triangle_outline(const brh_triangle* triangle, uint32_t color);

/**
 * @brief Draws a solid-colored filled triangle.
 *
 * Uses a scanline rasterization algorithm (flat-top/flat-bottom decomposition)
 * to fill the triangle's area with a single color.
 *
 * @param triangle Pointer to the triangle data. Coordinates might be modified
 *                 internally for sorting, hence not const.
 * @param color The 32-bit color (e.g., ARGB) to fill the triangle with.
 */
void draw_filled_triangle(brh_triangle* triangle, uint32_t color);

/**
 * @brief Draws a textured triangle.
 *
 * Uses scanline rasterization with barycentric interpolation of texture
 * coordinates to map a texture onto the triangle's area.
 * Note: Basic implementation uses linear interpolation, not perspective-correct.
 *
 * @param triangle Pointer to the triangle data. Coordinates might be modified
 *                 internally for sorting, hence not const.
 * @param texture Pointer to the loaded texture data (array of uint32_t colors).
 */
void draw_textured_triangle(brh_triangle* triangle, uint32_t* texture);

/**
 * @brief Calculates the barycentric coordinates of a point P relative to a triangle ABC.
 *
 * Determines the weights (alpha, beta, gamma) such that P = alpha*A + beta*B + gamma*C.
 * These weights are used for interpolation across the triangle surface.
 * Returns {0, 0, 0} for degenerate triangles.
 *
 * @param p The 2D point (screen coordinates) to find coordinates for.
 * @param a The 2D position of the first triangle vertex.
 * @param b The 2D position of the second triangle vertex.
 * @param c The 2D position of the third triangle vertex.
 * @return A `brh_vector3` containing the barycentric weights {alpha, beta, gamma}.
 */
brh_vector3 calculate_barycentic_coordinates(brh_vector2 p, brh_vector2 a, brh_vector2 b, brh_vector2 c);