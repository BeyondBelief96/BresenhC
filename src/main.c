#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
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

    mesh_texture_data = (uint32_t*) REDBRICK_TEXTURE;
    texture_width = 64;
    texture_height = 64;

    //bool loaded = load_gltf("./assets/supermarine_spitfire/scene.gltf", &mesh);
    /*bool loaded = load_obj("./assets/pumpkin.obj", &mesh, true);
    if (!loaded)
    {
        fprintf(stderr, "Error loading OBJ file\n");
        return;
    }*/

    load_cube_mesh();
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

void update(void)
{
    uint32_t time_to_wait = FRAME_TARGET_TIME - (uint32_t)(SDL_GetTicks() - previous_frame_time);

    // Only delay execution if we are running too fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    previous_frame_time = (uint32_t)SDL_GetTicks();
    mesh.rotation.y += 0.01f;
    mesh.translation.z = 5.0f;

    brh_mat4 world_matrix = mat4_create_world_matrix(mesh.translation, mesh.rotation, mesh.scale);
    triangles_to_render = NULL;
    int num_faces = array_length(mesh.faces);

    for (int i = 0; i < num_faces; i++)
    {
        brh_face face = mesh.faces[i];
        brh_vector3 face_vertices_model[3]; // Model space vertices
        brh_vector4 face_vertices_world[3]; // World space vertices (needed for depth sort)
        brh_vertex triangle_vertices[3];   // Final vertices for the triangle struct
        brh_texel face_texels[3];          // Texture coordinates

        face_vertices_model[0] = mesh.vertices[face.a - 1];
        face_vertices_model[1] = mesh.vertices[face.b - 1];
        face_vertices_model[2] = mesh.vertices[face.c - 1];

        face_texels[0] = face.texel_a;
        face_texels[1] = face.texel_b;
        face_texels[2] = face.texel_c;

        bool skip_triangle = false; // Flag to skip if w is near zero

        for (int j = 0; j < 3; j++)
        {
            // --- World Transformation ---
            brh_vector4 world_vertex = vec4_from_vec3(face_vertices_model[j]);
            mat4_mul_vec4_ref(&world_matrix, &world_vertex);
            face_vertices_world[j] = world_vertex; // Store for depth sorting

            // --- Projection to Clip Space ---
            brh_vector4 clip_space_vertex = mat4_mul_vec4(&perspective_projection_matrix, world_vertex);

            // --- Store Inverse W (Crucial!) ---
            // Store 1/w for perspective correction BEFORE division
            float original_w = clip_space_vertex.w;
            triangle_vertices[j].inv_w = 1.0f / original_w;

            // --- Perspective Division (Clip Space -> NDC) ---
            brh_vector4 ndc_vertex = clip_space_vertex;
            ndc_vertex.x *= triangle_vertices[j].inv_w; // x' = x/w
            ndc_vertex.y *= triangle_vertices[j].inv_w; // y' = y/w
            ndc_vertex.z *= triangle_vertices[j].inv_w; // z' = z/w (normalized depth)

            // --- Viewport Transformation (NDC -> Screen Space) ---
            // Convert x,y from [-1, 1] to [0, window_width/window_height]
            // Invert Y axis: (1 - ndc.y) maps [-1, 1] to [2, 0], then multiply by 0.5 * height.
            triangle_vertices[j].position.x = (ndc_vertex.x + 1.0f) * 0.5f * (float)window_width;
            triangle_vertices[j].position.y = (1.0f - ndc_vertex.y) * 0.5f * (float)window_height; // Y is inverted
            triangle_vertices[j].position.z = face_vertices_world[j].z; // Use original world Z for simple depth sorting
            // Or use ndc_vertex.z for Z-buffer: triangle_vertices[j].position.z = ndc_vertex.z;
            triangle_vertices[j].position.w = original_w; // Store original W for potential future use (or debugging)

            // --- Assign Texel ---
            triangle_vertices[j].texel = face_texels[j];
        }

        // --- Backface Culling (using world space vertices) ---
        brh_vector3 vecA_world = vec3_from_vec4(face_vertices_world[0]);
        brh_vector3 vecB_world = vec3_from_vec4(face_vertices_world[1]);
        brh_vector3 vecC_world = vec3_from_vec4(face_vertices_world[2]);
        brh_vector3 face_normal = get_face_normal(vecA_world, vecB_world, vecC_world);

        if (cull_method == CULL_BACKFACE)
        {
            // View vector from a vertex on the face towards the camera origin (in world space)
            brh_vector3 view_vector = vec3_subtract(camera_position, vecA_world);
            float angle_dot_product = vec3_dot(face_normal, view_vector);
            if (angle_dot_product < 0)
            {
                continue; // Skip back-facing triangle
            }
        }

        // Assign the calculated normal and color to all vertices for flat shading
        uint32_t triangle_color = calculate_flat_shading_color(face_normal, face.color);
        triangle_vertices[0].normal = face_normal;
        triangle_vertices[1].normal = face_normal;
        triangle_vertices[2].normal = face_normal;


        // --- Assemble and Store Triangle ---
        brh_triangle projected_triangle = {
            .vertices = { triangle_vertices[0], triangle_vertices[1], triangle_vertices[2] },
            .color = triangle_color,
            .avg_depth = (face_vertices_world[0].z + face_vertices_world[1].z + face_vertices_world[2].z) / 3.0f // Use world Z avg
        };

        array_push(triangles_to_render, projected_triangle);
    }

	// Sort the triangles to render by their average depth, implementing the painter's algorithm
    if (triangles_to_render != NULL)
    {
        int num_triangles = array_length(triangles_to_render);
        for (int i = 0; i < num_triangles; i++)
        {
            for (int j = i; j < num_triangles; j++)
            {
                if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
                {
                    brh_triangle temp = triangles_to_render[i];
                    triangles_to_render[i] = triangles_to_render[j];
                    triangles_to_render[j] = temp;
                }
            }
        }
    }
    
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
