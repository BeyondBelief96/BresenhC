#pragma once

#include <stdint.h>
#include "brh_vector.h" 
#include "brh_texture_manager.h"

/*
* @struct brh_texel
* 
* @brief Represents a 2D texture coordinate (u, v).
* 
* This structure is used to store texture coordinates for a vertex in a triangle.
* 
* @var brh_texel::u
* The horizontal coordinate of the texture (0.0 to 1.0).
* 
* @var brh_texel::v
* The vertical coordinate of the texture (0.0 to 1.0).
* 
* @note The coordinates are typically in the range [0.0, 1.0], but can be outside this range for texture wrapping.
*/
typedef struct brh_texel {
    float u;
    float v;
} brh_texel;


// Structure to hold perspective-correct attributes for interpolation
typedef struct {
    float inv_w; // 1/w

    // Texture coordinates
    float u_over_w;
    float v_over_w;

    // Gouraud shading (color components)
    float r_over_w;
    float g_over_w;
    float b_over_w;
    // We don't need alpha over w, usually alpha is constant or handled separately

    // Phong shading (normal components)
    float nx_over_w;
    float ny_over_w;
    float nz_over_w;

} brh_perspective_attribs;

/**
 * @struct brh_vertex
 * @brief Represents a vertex with position, texture coordinates, normal, and perspective attribute.
 * // ... existing comments ...
 * @var brh_vertex::position
 * A `brh_vector4` storing the final screen-space X and Y. The Z component might store
 * NDC Z (for depth buffering) or original world Z (for painter's algorithm).
 * The W component here is often not directly used after perspective division but might retain the original clip-space W for reference.
 * @var brh_vertex::texel
 * A `brh_texel` representing the (u, v) texture coordinates for the vertex.
 * @var brh_vertex::normal
 * A `brh_vector3` representing the *world-space* normal vector (used for lighting calculations before rasterization).
 * @var brh_vertex::color
 * A `uint32_t` storing the calculated vertex color (used *only* for Gouraud shading setup).
 * @var brh_vertex::inv_w
 * The inverse (1/w) of the vertex's W component in *clip space* (before perspective division).
 * This is crucial for perspective-correct interpolation.
 */
typedef struct {
    brh_vector4 position; // Holds final screen X, Y. Z/W usage depends on depth/sorting method.
    brh_texel texel;
    brh_vector3 normal;    // World-space normal for lighting calculations
    uint32_t color;        // Calculated vertex color (used for Gouraud setup)
    float inv_w;           // Inverse W (1/w) from clip space
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
void draw_textured_triangle(brh_triangle* triangle, brh_texture_handle texture);

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

/*
* @brief Interpolates between two vertices based on a parameter t.
* 
* This function linearly interpolates the position, texture coordinates,
* and normal of two vertices based on the parameter t.
* 
* @param v0 The first vertex.
* @param v1 The second vertex.
* @param t The interpolation parameter (0 <= t <= 1).
* 
* @return A new `brh_vertex` that is the result of the interpolation.
*/
brh_vertex interpolate_vertices(brh_vertex v0, brh_vertex v1, float t);