#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "upng.h"
#include "math_utils.h"
#include "display.h"
#include "brh_triangle.h"
#include "brh_face.h"
#include "brh_vector.h"
#include "brh_matrix.h"
#include "brh_mesh.h"
#include "array.h"
#include "model_loader.h"
#include "brh_light.h"

uint32_t previous_frame_time = 0;
brh_triangle* triangles_to_render = NULL;

bool is_running = true;
uint32_t cell_size;

brh_vector3 camera_position = { .x = 0, .y = 0, .z = 0 };
brh_mat4 perspective_projection_matrix;

void setup(void)
{
    render_method = RENDER_WIREFRAME;
    cull_method = CULL_BACKFACE;

    // Allocate back buffers
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
    assert(color_buffer != NULL); // Ensure malloc succeeded
    if (!color_buffer)
    {
        assert(color_buffer);
        fprintf(stderr, "Color buffer could not be created!");
        return;
    }

    // Create color buffer texture
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
    if (!color_buffer_texture)
    {
        fprintf(stderr, "Color buffer texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return;
    }
    
    cell_size = gcd(window_width, window_height);\

	perspective_projection_matrix = mat4_create_perspective_projection(degrees_to_radians(60.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);

    // Manually load the hard-coded redbrick texture data
    /*mesh_texture_data = (uint32_t*) REDBRICK_TEXTURE;
    texture_width = 64;
    texture_height = 64;*/

    //bool loaded = load_gltf("./assets/supermarine_spitfire/scene.gltf", &mesh);
    bool loaded = load_obj("./assets/cube.obj", &mesh, true);
    if (!loaded)
    {
        fprintf(stderr, "Error loading OBJ file\n");
        return;
    }

    //load_cube_mesh();

    bool texture_loaded = load_png_texture_data("./assets/tnt.png");
    if (!texture_loaded)
    {
        fprintf(stderr, "Error loading PNG texture\n");
        return;
    }
}

void process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_EVENT_QUIT:
        is_running = false;
        break;
    case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_ESCAPE)
        {
            is_running = false;
        }
        if (event.key.key == SDLK_1)
        {
            render_method = RENDER_WIREFRAME_VERTEX;
        }
        if (event.key.key == SDLK_2)
        {
            render_method = RENDER_WIREFRAME;
        }
        if (event.key.key == SDLK_3)
        {
            render_method = RENDER_FILL_TRIANGLE;
        }
        if (event.key.key == SDLK_4)
        {
            render_method = RENDER_FILL_TRIANGLE_WIREFRAME;
        }
        if (event.key.key == SDLK_5)
        {
            render_method = RENDER_TEXTURED;
        }
		if (event.key.key == SDLK_6)
		{
			render_method = RENDER_TEXTURED_WIREFRAME;
		}
        if (event.key.key == SDLK_C)
        {
            cull_method = CULL_BACKFACE;
        }
        if (event.key.key == SDLK_D)
        {
            cull_method = CULL_NONE;
        }

        break;
    }
}

/**
 * @brief Transform face vertices through the rendering pipeline
 *
 * @param face_vertices_model Model space vertices
 * @param face_vertices_world Output world space vertices
 * @param triangle_vertices Output processed vertices
 * @param face_texels Texture coordinates
 * @param world_matrix World transformation matrix
 * @return bool false if face should be culled, true otherwise
 */
static bool transform_face_vertices(brh_vector3 face_vertices_model[3],
    brh_vector4 face_vertices_world[3],
    brh_vertex triangle_vertices[3],
    brh_texel face_texels[3],
    brh_mat4 world_matrix)
{
    for (int j = 0; j < 3; j++) {
        // --- World Transformation ---
        brh_vector4 world_vertex = vec4_from_vec3(face_vertices_model[j]);
        mat4_mul_vec4_ref(&world_matrix, &world_vertex);
        face_vertices_world[j] = world_vertex;

        // --- Projection to Clip Space ---
        brh_vector4 clip_space_vertex = mat4_mul_vec4(&perspective_projection_matrix, world_vertex);

        // --- Store Inverse W for perspective correction ---
        float original_w = clip_space_vertex.w;
        triangle_vertices[j].inv_w = 1.0f / original_w;

        // --- Perspective Division (Clip Space -> NDC) ---
        brh_vector4 ndc_vertex = clip_space_vertex;
        ndc_vertex.x *= triangle_vertices[j].inv_w;
        ndc_vertex.y *= triangle_vertices[j].inv_w;
        ndc_vertex.z *= triangle_vertices[j].inv_w;

        // --- Viewport Transformation (NDC -> Screen Space) ---
        triangle_vertices[j].position.x = (ndc_vertex.x + 1.0f) * 0.5f * (float)window_width;
        triangle_vertices[j].position.y = (1.0f - ndc_vertex.y) * 0.5f * (float)window_height;
        triangle_vertices[j].position.z = face_vertices_world[j].z; // Store world Z for depth sorting
        triangle_vertices[j].position.w = original_w;

        // --- Assign Texture Coordinates ---
        triangle_vertices[j].texel = face_texels[j];
    }

    // --- Backface Culling ---
    if (cull_method == CULL_BACKFACE) {
        brh_vector3 vecA_world = vec3_from_vec4(face_vertices_world[0]);
        brh_vector3 vecB_world = vec3_from_vec4(face_vertices_world[1]);
        brh_vector3 vecC_world = vec3_from_vec4(face_vertices_world[2]);
        brh_vector3 face_normal = get_face_normal(vecA_world, vecB_world, vecC_world);

        brh_vector3 view_vector = vec3_subtract(camera_position, vecA_world);
        float angle_dot_product = vec3_dot(face_normal, view_vector);

        if (angle_dot_product < 0) {
            return false; // Cull this triangle
        }
    }

    return true;
}

/**
 * @brief Process all faces in a mesh and prepare them for rendering
 *
 * @param mesh The mesh to process
 * @param world_matrix The world transformation matrix
 */
void process_mesh_faces(brh_mesh* mesh, brh_mat4 world_matrix)
{
    int num_faces = array_length(mesh->faces);
    int num_texcoords = array_length(mesh->texcoords);

    for (int i = 0; i < num_faces; i++) {
        brh_face face = mesh->faces[i];
        brh_vector3 face_vertices_model[3]; // Model space vertices
        brh_vector4 face_vertices_world[3]; // World space vertices
        brh_vertex triangle_vertices[3];    // Final vertices
        brh_texel face_texels[3] = { {0, 0}, {0, 0}, {0, 0} }; // Default to 0,0

        // Get vertex positions
        face_vertices_model[0] = mesh->vertices[face.a];
        face_vertices_model[1] = mesh->vertices[face.b];
        face_vertices_model[2] = mesh->vertices[face.c];

        // Get texture coordinates if available
        if (num_texcoords > 0) {
            if (face.a_vt < num_texcoords) face_texels[0] = mesh->texcoords[face.a_vt];
            if (face.b_vt < num_texcoords) face_texels[1] = mesh->texcoords[face.b_vt];
            if (face.c_vt < num_texcoords) face_texels[2] = mesh->texcoords[face.c_vt];
        }

        // Transform vertices and check backface culling
        if (!transform_face_vertices(face_vertices_model, face_vertices_world,
            triangle_vertices, face_texels,
            world_matrix)) {
            continue; // Skip this face (culled)
        }

        // Calculate face normal and lighting
        brh_vector3 vecA_world = vec3_from_vec4(face_vertices_world[0]);
        brh_vector3 vecB_world = vec3_from_vec4(face_vertices_world[1]);
        brh_vector3 vecC_world = vec3_from_vec4(face_vertices_world[2]);
        brh_vector3 face_normal = get_face_normal(vecA_world, vecB_world, vecC_world);

        // Apply flat shading
        uint32_t triangle_color = calculate_flat_shading_color(face_normal, face.color);

        // Set normal for all vertices (flat shading)
        triangle_vertices[0].normal = face_normal;
        triangle_vertices[1].normal = face_normal;
        triangle_vertices[2].normal = face_normal;

        // Create and store the triangle
        float avg_depth = (face_vertices_world[0].z + face_vertices_world[1].z + face_vertices_world[2].z) / 3.0f;
        brh_triangle projected_triangle = {
            .vertices = { triangle_vertices[0], triangle_vertices[1], triangle_vertices[2] },
            .color = triangle_color,
            .avg_depth = avg_depth
        };

        array_push(triangles_to_render, projected_triangle);
    }
}

/**
 * @brief Sort triangles by depth for rendering
 *
 * @param triangles Array of triangles to sort
 */
void sort_triangles(brh_triangle* triangles)
{
    if (triangles == NULL) return;

    int num_triangles = array_length(triangles);

    // Simple bubble sort (can be replaced with a faster algorithm)
    for (int i = 0; i < num_triangles; i++) {
        for (int j = i; j < num_triangles; j++) {
            if (triangles[i].avg_depth < triangles[j].avg_depth) {
                brh_triangle temp = triangles[i];
                triangles[i] = triangles[j];
                triangles[j] = temp;
            }
        }
    }
}

void update(void)
{
    // Frame timing management
    uint32_t time_to_wait = FRAME_TARGET_TIME - (uint32_t)(SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    previous_frame_time = (uint32_t)SDL_GetTicks();

    // Update model transformations
    mesh.rotation.y += 0.01f;
    mesh.translation.z = 5.0f;

    // Generate world matrix
    brh_mat4 world_matrix = mat4_create_world_matrix(mesh.translation, mesh.rotation, mesh.scale);

    // Reset triangle buffer
    triangles_to_render = NULL;

    // Process each face in the mesh
    process_mesh_faces(&mesh, world_matrix);

    // Sort triangles for rendering (painter's algorithm)
    sort_triangles(triangles_to_render);
}

void render(void)
{
    clear_color_buffer(0xFF000000);
    draw_grid(cell_size, 0xFF333333);

    int num_triangles = array_length(triangles_to_render);

    for (int i = 0; i < num_triangles; i++)
    {
        brh_triangle triangle = triangles_to_render[i];

        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIREFRAME)
        {
            draw_filled_triangle(&triangle, triangle.color);
        }

        if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIREFRAME)
        {
            draw_textured_triangle(&triangle, mesh_texture_data);
        }

        if (render_method == RENDER_WIREFRAME || render_method == RENDER_WIREFRAME_VERTEX 
            || render_method == RENDER_FILL_TRIANGLE_WIREFRAME || render_method == RENDER_TEXTURED_WIREFRAME)
        {
            draw_triangle_outline(&triangle, 0xFFFFFFFF);
        }

        if (render_method == RENDER_WIREFRAME_VERTEX)
        {
            draw_rect((int)triangle.vertices[0].position.x - 3, (int)triangle.vertices[0].position.y - 3, 6, 6, 0xFFFF0000);
            draw_rect((int)triangle.vertices[1].position.x - 3, (int)triangle.vertices[1].position.y - 3, 6, 6, 0xFFFF0000);
            draw_rect((int)triangle.vertices[2].position.x - 3, (int)triangle.vertices[2].position.y - 3, 6, 6, 0xFFFF0000);
        }
    }

    // Clear the array of triangles to render every frame loop
    array_free(triangles_to_render);

    render_color_buffer();

    SDL_RenderPresent(renderer);
}

void free_resources(void)
{
    upng_free(png_texture);
    free(color_buffer);
    array_free(mesh.vertices);
    array_free(mesh.faces);
}

int main(int argc, char* argv[])
{
    is_running = initialize_window();
    setup();

    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
