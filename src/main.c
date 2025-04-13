#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "upng.h"
#include "math_utils.h"
#include "brh_display.h"
#include "brh_triangle.h"
#include "brh_face.h"
#include "brh_vector.h"
#include "brh_matrix.h"
#include "brh_mesh.h"
#include "array.h"
#include "model_loader.h"
#include "brh_light.h"
#include "brh_camera.h"
#include "brh_clipping.h"
#include "brh_geometry.h"

/* --------- Global Variables --------- */
bool is_running = true;
float delta_time_seconds = 0.0f;
uint32_t previous_frame_time = 0;
uint32_t cell_size = 0;

/* ------- Mouse Camera Parameters -------*/
int mouse_x_prev = 0;
int mouse_y_prev = 0;
bool mouse_initialized = false;
bool mouse_locked = false;
int movement_forward = 0;  // -1 for backward, 1 for forward, 0 for none
int movement_right = 0;    // -1 for left, 1 for right, 0 for none
int movement_up = 0;       // -1 for down, 1 for up, 0 for none

/* Rendering transformation matrices */
brh_mat4 world_matrix;
brh_mat4 camera_matrix;
brh_mat4 perspective_projection_matrix;

/* Triangle buffer for rendering */
brh_triangle* triangles_to_render = NULL;
int triangles_to_render_capacity = 0;
int triangles_to_render_count = 0;

/* --------- Function Declarations --------- */
bool initialize_resources(void);
bool load_mesh_resources(void);
void initialize_mouse_camera(brh_mouse_camera* camera);
void process_input(void);
void update(void);
void render(void);
void cleanup_resources(void);
void cleanup_mesh_resources(void);

/* --------- Main Function --------- */
int main(int argc, char* argv[])
{
    /* Initialize resources */
    is_running = initialize_resources();
    if (!is_running) {
        fprintf(stderr, "Failed to initialize resources\n");
        cleanup_resources();
        return 1;
    }

    /* Main game loop */
    while (is_running) {
        process_input();
        update();
        render();
    }

    /* Cleanup and exit */
    cleanup_resources();
    return 0;
}

/* --------- Resource Initialization --------- */
bool initialize_resources(void)
{
    /* Initialize display resources (window, renderer, buffers) */
    if (!initialize_display_resources()) {
        fprintf(stderr, "Failed to initialize display resources\n");
        return false;
    }

    /* Set default rendering options */
    set_render_method(RENDER_WIREFRAME);
    set_cull_method(CULL_BACKFACE);

    /* Calculate grid cell size for background */
    cell_size = gcd(get_window_width(), get_window_height());

    /* --- Initialize projection matrix --- */
    float fov_radians = degrees_to_radians(frustum_fov_y);
    float aspect_ratio = (float)get_window_width() / (float)get_window_height();
    perspective_projection_matrix = mat4_create_perspective_projection(
        fov_radians,
        aspect_ratio,
        near_plane,
        far_plane
    );

    /* Initialize mouse camera */
    initialize_mouse_camera(&mouse_camera);
    SDL_SetWindowRelativeMouseMode(get_window(), false);

    /* Load mesh and texture resources */
    if (!load_mesh_resources()) {
        fprintf(stderr, "Failed to load mesh resources\n");
        return false;
    }

    /* Initialize camera parameters */
    lookat_camera.position = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    lookat_camera.target = (brh_vector3){ 0.0f, 0.0f, 5.0f };

    return true;
}

/* Helper to initialize the mouse camera */
void initialize_mouse_camera(brh_mouse_camera* camera)
{
    camera->position = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    camera->direction = (brh_vector3){ 0.0f, 0.0f, 1.0f };
    camera->yaw_angle = 0.0f;
    camera->pitch_angle = 0.0f;
    camera->speed = 5.0f;
    camera->sensitivity = 0.001f;
}

/* Helper to load mesh and related resources */
bool load_mesh_resources(void)
{
    /* Load 3D mesh */
    bool loaded = load_obj("./assets/f22.obj", &mesh, true);
    if (!loaded) {
        fprintf(stderr, "Error: Failed to load OBJ file\n");
        return false;
    }

    /* Allocate triangle buffer */
    int num_faces = array_length(mesh.faces);
    if (num_faces <= 0) {
        fprintf(stderr, "Error: Mesh loaded with zero faces\n");
        array_free(mesh.vertices);
        array_free(mesh.faces);
        array_free(mesh.texcoords);
        array_free(mesh.normals);
        return false;
    }

    triangles_to_render_capacity = num_faces;
    triangles_to_render = (brh_triangle*)malloc(sizeof(brh_triangle) * triangles_to_render_capacity);

    if (!triangles_to_render) {
        fprintf(stderr, "Error: Failed to allocate triangle render buffer\n");
        array_free(mesh.vertices);
        array_free(mesh.faces);
        array_free(mesh.texcoords);
        array_free(mesh.normals);
        return false;
    }

    /* Load texture */
    bool texture_loaded = load_png_texture_data("./assets/f22.png");
    if (!texture_loaded) {
        fprintf(stderr, "Error: Failed to load PNG texture\n");
        free(triangles_to_render);
        triangles_to_render = NULL;
        array_free(mesh.vertices);
        array_free(mesh.faces);
        array_free(mesh.texcoords);
        array_free(mesh.normals);
        return false;
    }

    return true;
}

/* --------- Input Processing --------- */
void process_input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            is_running = false;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                // Lock/unlock mouse on left click
                mouse_locked = !mouse_locked;
                SDL_SetWindowRelativeMouseMode(get_window(), mouse_locked);
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (mouse_locked) {
                // Only process mouse movement if mouse is locked
                int mouse_x_rel = (int)event.motion.xrel;
                int mouse_y_rel = (int)event.motion.yrel;

                if (!mouse_initialized) {
                    // Skip the first frame after mouse lock to avoid jumps
                    mouse_initialized = true;
                    break;
                }

                // Update camera orientation based on mouse movement
                update_mouse_camera_view(&mouse_camera, mouse_x_rel, mouse_y_rel, delta_time_seconds);
            }
            break;

        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key)
            {
            case SDLK_ESCAPE:
                is_running = false;
                break;
            case SDLK_1:
                set_render_method(RENDER_WIREFRAME_VERTEX);
                break;
            case SDLK_2:
                set_render_method(RENDER_WIREFRAME);
                break;
            case SDLK_3:
                set_render_method(RENDER_FILL_TRIANGLE);
                break;
            case SDLK_4:
                set_render_method(RENDER_FILL_TRIANGLE_WIREFRAME);
                break;
            case SDLK_5:
                set_render_method(RENDER_TEXTURED);
                break;
            case SDLK_6:
                set_render_method(RENDER_TEXTURED_WIREFRAME);
                break;
            case SDLK_C:
                set_cull_method(CULL_BACKFACE);
                break;
            case SDLK_X:
                set_cull_method(CULL_NONE);
                break;
                // Camera movement controls
            case SDLK_W:
                movement_forward = 1;
                break;
            case SDLK_S:
                movement_forward = -1;
                break;
            case SDLK_D:
                movement_right = 1;
                break;
            case SDLK_A:
                movement_right = -1;
                break;
            case SDLK_SPACE:
                movement_up = 1;
                break;
            case SDLK_LCTRL:
                movement_up = -1;
                break;
            }
            break;
        case SDL_EVENT_KEY_UP:
            switch (event.key.key)
            {
            case SDLK_W:
            case SDLK_S:
                movement_forward = 0;
                break;
            case SDLK_A:
            case SDLK_D:
                movement_right = 0;
                break;
            case SDLK_SPACE:
            case SDLK_LCTRL:
                movement_up = 0;
                break;
            }
            break;
        }
    }

    // Update camera position based on movement keys
    move_mouse_camera(&mouse_camera, movement_forward, movement_right, movement_up, delta_time_seconds);
}

/* --------- Update Game State --------- */
void update(void)
{
    /* ------- Frame Timing Management ------- */
    // Cap frame rate to target FPS
    uint32_t time_to_wait = FRAME_TARGET_TIME - ((uint32_t)SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    // Calculate delta time factor converted to seconds to update our game objects in a consistent manner.
    delta_time_seconds = (SDL_GetTicks() - previous_frame_time) / 1000.0f;
    previous_frame_time = (uint32_t)SDL_GetTicks();

    /* ------- Update Model Transform ------- */
    // Position model in front of camera
    mesh.translation.z = 5.0f;

    // Generate matrices for this frame
    world_matrix = mat4_create_world_matrix(mesh.translation, mesh.rotation, mesh.scale);
    camera_matrix = get_mouse_camera_view_matrix(&mouse_camera);

    /* ------- Process Mesh Faces ------- */
    // Reset the triangle count for this frame
    triangles_to_render_count = 0;

    int num_faces = array_length(mesh.faces);
    int num_texcoords = array_length(mesh.texcoords);
    int num_clipped_triangles = 0;

    // Temporary triangle buffer for clipping
    brh_triangle clipped_triangles[MAX_CLIPPED_TRIANGLES];

    // Process each face, transforming vertices and culling as needed
    for (int i = 0; i < num_faces && triangles_to_render_count < triangles_to_render_capacity; i++) {
        brh_face face = mesh.faces[i];
        brh_vector3 face_vertices_model[3];
        brh_vector4 face_vertices_world[3];
        brh_vector4 face_vertices_camera[3];
        brh_vertex triangle_vertices[3];
        brh_texel face_texels[3] = { {0, 0}, {0, 0}, {0, 0} };

        // Get vertices from mesh
        face_vertices_model[0] = mesh.vertices[face.a];
        face_vertices_model[1] = mesh.vertices[face.b];
        face_vertices_model[2] = mesh.vertices[face.c];

        // Get texture coordinates if available
        if (num_texcoords > 0) {
            if (face.a_vt < num_texcoords) face_texels[0] = mesh.texcoords[face.a_vt];
            if (face.b_vt < num_texcoords) face_texels[1] = mesh.texcoords[face.b_vt];
            if (face.c_vt < num_texcoords) face_texels[2] = mesh.texcoords[face.c_vt];
        }

        /* --- Transform vertices through the rendering pipeline --- */

        for (int j = 0; j < 3; j++) {
            // Transform from model space to world space
            brh_vector4 world_vertex = vec4_from_vec3(face_vertices_model[j]);
            mat4_mul_vec4_ref(&world_matrix, &world_vertex);
            face_vertices_world[j] = world_vertex;

            // Transform from world space to camera (view) space
            brh_vector4 camera_vertex = face_vertices_world[j];
            mat4_mul_vec4_ref(&camera_matrix, &camera_vertex);
            face_vertices_camera[j] = camera_vertex;

            // Transform from camera space to clip space
            brh_vector4 clip_vertex = mat4_mul_vec4(
                &perspective_projection_matrix,
                camera_vertex
            );

            // Store inverse W for perspective correction
            float original_w = clip_vertex.w;
            triangle_vertices[j].inv_w = 1.0f / original_w;

            // Save the clip space position in the vertex (for clipping)
            triangle_vertices[j].position = clip_vertex;

            // Assign texture coordinates
            triangle_vertices[j].texel = face_texels[j];
        }

        /* --- Backface Culling --- */
        if (get_cull_method() == CULL_BACKFACE) {
            brh_vector3 vecA_camera = vec3_from_vec4(face_vertices_camera[0]);
            brh_vector3 vecB_camera = vec3_from_vec4(face_vertices_camera[1]);
            brh_vector3 vecC_camera = vec3_from_vec4(face_vertices_camera[2]);
            brh_vector3 face_normal = get_face_normal(vecA_camera, vecB_camera, vecC_camera);

            // Calculate view vector (from face to camera)
            brh_vector3 origin = { 0.0f, 0.0f, 0.0f };
            brh_vector3 view_vector = vec3_subtract(origin, vecA_camera);
            float angle_dot_product = vec3_dot(face_normal, view_vector);

            if (angle_dot_product < 0) {
                continue; // Skip this face if it's backfacing
            }
        }

        /* --- Add triangle to render buffer if visible --- */
        // Calculate face normal for lighting
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

        // Create triangle
        brh_triangle clip_space_triangle = {
            .vertices = { triangle_vertices[0], triangle_vertices[1], triangle_vertices[2] },
            .color = triangle_color,
        };

        // Clip the triangle against the clipping planes
        num_clipped_triangles = clip_triangle(&clip_space_triangle, clipped_triangles);

        // Process each clipped triangle (could be 0, 1, or multiple)
        for (int j = 0; j < num_clipped_triangles && triangles_to_render_count < triangles_to_render_capacity; j++) {
            // Create a projected triangle from the clipped triangle
            brh_triangle triangle_to_render = clipped_triangles[j];

            // Perform perspective division and viewport transformation for each vertex
            for (int k = 0; k < 3; k++) {
                // Get original clip space position
                brh_vector4 clip_pos = triangle_to_render.vertices[k].position;

                // Perform perspective division (clip space to NDC)
                float inv_w = 1.0f / clip_pos.w;
                brh_vector4 ndc_vertex;
                ndc_vertex.x = clip_pos.x * inv_w;
                ndc_vertex.y = clip_pos.y * inv_w;
                ndc_vertex.z = clip_pos.z * inv_w;
                ndc_vertex.w = clip_pos.w;

                // Update the vertex with viewport transformation (NDC to screen space)n 
                triangle_to_render.vertices[k].position.x = (ndc_vertex.x + 1.0f) * 0.5f * (float)get_window_width();
                triangle_to_render.vertices[k].position.y = (1.0f - ndc_vertex.y) * 0.5f * (float)get_window_height();
                triangle_to_render.vertices[k].position.z = ndc_vertex.z; // Now in NDC space [-1,1]
                triangle_to_render.vertices[k].position.w = clip_pos.w;   // Preserve W for later use
                triangle_to_render.vertices[k].inv_w = inv_w;             // Update inv_w for perspective correction
            }

            // Add the projected triangle to the render buffer
            triangles_to_render[triangles_to_render_count++] = triangle_to_render;
        }

        // Check if we've hit capacity and need to stop processing
        if (triangles_to_render_count >= triangles_to_render_capacity) {
            fprintf(stderr, "Warning: Triangle buffer filled to capacity (%d triangles)\n",
                triangles_to_render_capacity);
            break; // Exit the face processing loop
        }
    }
}

/* --------- Render the Scene --------- */
void render(void)
{
    const enum render_method current_render_method = get_render_method();

    /* Clear buffers */
    clear_color_buffer(0xFF000000);  // Black background
    clear_z_buffer();

    /* Draw background grid */
    draw_grid(cell_size, 0xFF333333);  // Dark gray grid

    /* Render all triangles */
    for (int i = 0; i < triangles_to_render_count; i++) {
        brh_triangle* triangle = &triangles_to_render[i];

        /* Draw filled triangles if required */
        if (current_render_method == RENDER_FILL_TRIANGLE ||
            current_render_method == RENDER_FILL_TRIANGLE_WIREFRAME) {
            draw_filled_triangle(triangle, triangle->color);
        }

        /* Draw textured triangles if required */
        if (current_render_method == RENDER_TEXTURED ||
            current_render_method == RENDER_TEXTURED_WIREFRAME) {
            draw_textured_triangle(triangle, mesh_texture_data);
        }

        /* Draw wireframe if required */
        if (current_render_method == RENDER_WIREFRAME ||
            current_render_method == RENDER_WIREFRAME_VERTEX ||
            current_render_method == RENDER_FILL_TRIANGLE_WIREFRAME ||
            current_render_method == RENDER_TEXTURED_WIREFRAME) {
            draw_triangle_outline(triangle, 0xFFFFFFFF);  // White wireframe
        }

        /* Draw vertex markers if required */
        if (current_render_method == RENDER_WIREFRAME_VERTEX) {
            // Draw red squares at each vertex
            for (int j = 0; j < 3; j++) {
                draw_rect(
                    (int)triangle->vertices[j].position.x - 3,
                    (int)triangle->vertices[j].position.y - 3,
                    6, 6, 0xFFFF0000
                );
            }
        }
    }

    /* Present the frame */
    render_color_buffer();
    SDL_RenderPresent(get_renderer());
}

/* --------- Mesh Resource Cleanup --------- */
void cleanup_mesh_resources(void)
{
    if (triangles_to_render) {
        free(triangles_to_render);
        triangles_to_render = NULL;
    }

    if (mesh.vertices) {
        array_free(mesh.vertices);
        mesh.vertices = NULL;
    }

    if (mesh.faces) {
        array_free(mesh.faces);
        mesh.faces = NULL;
    }

    if (mesh.texcoords) {
        array_free(mesh.texcoords);
        mesh.texcoords = NULL;
    }

    if (mesh.normals) {
        array_free(mesh.normals);
        mesh.normals = NULL;
    }
}

/* --------- Resource Cleanup --------- */
void cleanup_resources(void)
{
    // Free mesh resources
    cleanup_mesh_resources();

    // Clean up texture
    if (png_texture) {
        upng_free(png_texture);
        png_texture = NULL;
    }

    // Clean up display resources
    cleanup_display_resources();
}