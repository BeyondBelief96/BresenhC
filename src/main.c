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

uint32_t previous_frame_time = 0;
brh_triangle* triangles_to_render = NULL;

bool is_running = true;
uint32_t cell_size;

float fov_factor = 640;

brh_vector3 camera_position = { .x = 0, .y = 0, .z = 0 };

brh_vector2 project(brh_vector3 point)
{
    brh_vector2 projected_point = {
        .x = ((point.x / point.z) * fov_factor),
        .y = ((point.y / point.z) * fov_factor),
    };

    return projected_point;
}

bool should_cull_face_manual(brh_vector3* vertices, brh_vector3 camera_position)
{
    // Check for backface culling
    /*
    *     A
    *    / \
    *  C-----B
    */
    brh_vector3 vecA = vertices[0];
    brh_vector3 vecB = vertices[1];
    brh_vector3 vecC = vertices[2];
    brh_vector3 vecAB = vec3_subtract(vecB, vecA);
    brh_vector3 vecAC = vec3_subtract(vecC, vecA);
    brh_vector3 face_normal = vec3_cross(vecAB, vecAC);
    // Goes from vector a on the face to the camera position
    brh_vector3 view_vector = vec3_subtract(camera_position, vecA);
    float angle_between = vec3_dot(face_normal, view_vector);
    if (angle_between < 0)
    {
        return true;
    }
    return false;
}

bool should_cull_face(brh_vector3 face_normal, brh_vector3 camera_position)
{
    brh_vector3 view_vector = vec3_subtract(camera_position, face_normal);
    float angle_between = vec3_dot(face_normal, view_vector);
    return angle_between < 0;
}

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

    cell_size = gcd(window_width, window_height);

    //bool loaded = load_gltf("./assets/supermarine_spitfire/scene.gltf", &mesh);
    /*bool loaded = load_obj("./assets/f22.obj", &mesh, true);
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

    mesh.scale.x += 0.002f;

    // Create a scale matrix that will be used to multiplay the mesh vertices
	brh_mat4 scale_matrix = mat4_create_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);

    mesh.rotation.x += 0.01f;
    mesh.rotation.y += 0.01f;
    mesh.rotation.z += 0.01f;

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

            // Preform rotations

            //TODO: Use a matrix to scale our original vertices
			mat4_mul_vec4(scale_matrix, transformed_vertex);

            /*transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);*/

            // Translate the vertex away from the camera by the camera position
            transformed_vertex.z += 5;

            transformed_vertices[j] = transformed_vertex;
        }

        if (cull_method == CULL_BACKFACE)
        {
            brh_vector3 transformed_vertices_vec3[3];
            for (int j = 0; j < 3; j++)
            {
                transformed_vertices_vec3[j] = vec3_from_vec4(transformed_vertices[j]);
            }

            if (should_cull_face_manual(transformed_vertices_vec3, camera_position))
            {
                continue;
            }
        }

        brh_vector2 projected_points[3];
        for (int j = 0; j < 3; j++)
        {
            // Project the vertex from 3D World space to 2D screen space
			projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));

            // Scale and translate the projected point to the center of the screen
            projected_points[j].x += window_width / 2;
            projected_points[j].y += window_height / 2;
        }

        brh_triangle projected_triangle = {
            .points[0] = projected_points[0],
            .points[1] = projected_points[1],
            .points[2] = projected_points[2],
            .color = face.color,
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
