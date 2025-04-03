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

    //bool loaded = load_gltf("./assets/supermarine_spitfire/scene.gltf", &mesh);
    bool loaded = load_obj("./assets/f22.obj", &mesh, true);
    /*if (!loaded)
    {
        fprintf(stderr, "Error loading OBJ file\n");
        return;
    }*/

    //load_cube_mesh();
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

    mesh.rotation.x += 0.01f;
    mesh.rotation.y += 0.01f;
    mesh.rotation.z += 0.01f;
    mesh.translation.z = 10.0f;

    // Create the world matrix to transform each vertex of the mesh
	brh_mat4 world_matrix = mat4_create_world_matrix(mesh.translation, mesh.rotation, mesh.scale);

    // Initialize the array of triangles to render
    triangles_to_render = NULL;

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++)
    {
        brh_face face = mesh.faces[i];
        brh_vector3 face_vertices[3];
        face_vertices[0] = mesh.vertices[face.a - 1];
        face_vertices[1] = mesh.vertices[face.b - 1];
        face_vertices[2] = mesh.vertices[face.c - 1];

        brh_vector4 transformed_vertices[3];
        for (int j = 0; j < 3; j++)
        {
            brh_vector4 transformed_vertex = vec4_from_vec3(face_vertices[j]);
			mat4_mul_vec4_ref(&world_matrix, &transformed_vertex);
            transformed_vertices[j] = transformed_vertex;
        }

        // Backface culling test

        // Check for backface culling
        /*
        *     A
        *    / \
        *  C-----B
        */
        brh_vector3 vecA = vec3_from_vec4(transformed_vertices[0]);
        brh_vector3 vecB = vec3_from_vec4(transformed_vertices[1]);
        brh_vector3 vecC = vec3_from_vec4(transformed_vertices[2]);
		brh_vector3 face_normal = get_face_normal(vecA, vecB, vecC);
        // Goes from vector a on the face to the camera position
        if (cull_method == CULL_BACKFACE)
        {
            brh_vector3 view_vector = vec3_subtract(camera_position, vecA);
            float angle_between = vec3_dot(face_normal, view_vector);
            if (angle_between < 0)
            {
                continue;
            }
        }

        // Calculate the triangle's color based on the global light direction
        uint32_t triangle_color = calculate_flat_shading_color(face_normal, face.color);

        brh_vector4 projected_points[3];
        for (int j = 0; j < 3; j++)
        {
            // Project the vertex from 3D World space to 2D screen space
			projected_points[j] = mat4_project_vec4(&perspective_projection_matrix, &transformed_vertices[j]);

			// Scale the points based on the window dimensions and invert the x and y coordinates to match the screen space
			projected_points[j].x *= -(float)window_width / 2.0f;
			projected_points[j].y *= -(float)window_height / 2.0f;
            

            // Translate the projected points to the middle of the screen
            projected_points[j].x += (window_width / 2.0f);
            projected_points[j].y += (window_height / 2.0f);
        }

        brh_triangle projected_triangle = {
            .points = {
                { projected_points[0].x, projected_points[0].y },
                { projected_points[1].x, projected_points[1].y },
                { projected_points[2].x, projected_points[2].y },
            },
            .color = triangle_color,
			.avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z)
        };

        // Save each projected triangle for each face
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
            draw_filled_triangle((int)triangle.points[0].x, (int)triangle.points[0].y, (int)triangle.points[1].x, (int)triangle.points[1].y, (int)triangle.points[2].x, (int)triangle.points[2].y, triangle.color);
        }

        if (render_method == RENDER_WIREFRAME || render_method == RENDER_WIREFRAME_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIREFRAME)
        {
            draw_triangle_outline(triangle, 0xFFFFFFFF);
        }

        if (render_method == RENDER_WIREFRAME_VERTEX)
        {
            draw_rect((int)triangle.points[0].x - 3, (int)triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect((int)triangle.points[1].x - 3, (int)triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect((int)triangle.points[2].x - 3, (int)triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
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
