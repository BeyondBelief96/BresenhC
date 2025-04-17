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
#include "brh_vector.h"
#include "brh_matrix.h"
#include "brh_light.h"
#include "brh_camera.h"
#include "brh_geometry.h"
#include "brh_renderable.h"

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

#define MAX_NUM_RENDERABLES 32

/* Rendering transformation matrices */
brh_mat4 camera_matrix;
brh_mat4 perspective_projection_matrix;

brh_look_at_camera* lookat_camera = NULL;
brh_mouse_camera* mouse_camera = NULL;

brh_renderable_handle f117_renderable = NULL;
brh_renderable_handle f22_renderable = NULL;
brh_renderable_handle mirage_renderable = NULL;
brh_renderable_handle crab_renderable = NULL;
brh_renderable_handle drone_renderable = NULL;

brh_renderable_handle renderables[MAX_NUM_RENDERABLES];

/* --------- Function Declarations --------- */
bool initialize_resources(void);
bool load_mesh_resources(void);
void initialize_global_light(void);
void process_input(void);
void update(void);
void render(void);
void cleanup_resources(void);
void cleanup_camera_resources(void);
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
    float fov_radians = degrees_to_radians(get_frustum_fov_y());
    float aspect_ratio = get_aspect_ratio();
    perspective_projection_matrix = mat4_create_perspective_projection(
        fov_radians,
        aspect_ratio,
        get_frustum_near_plane(),
        get_frustum_far_plane()
    );

    /* Load mesh and texture resources */
    if (!load_mesh_resources()) {
        fprintf(stderr, "Failed to load mesh resources\n");
        return false;
    }

    /* Initialize camera parameters */
    set_frustum_parameters(60.0f, 1.0f, 100.0f);

    // Create lookat camera
    lookat_camera = create_look_at_camera((brh_vector3) {0.0f, 0.0f, 0.0f},(brh_vector3) {0.0f, 0.0f, 5.0f});

    // Create mouse camera
    mouse_camera = create_mouse_camera((brh_vector3) {0.0f, 0.0f, 0.0f}, (brh_vector3) {0.0f, 0.0f, 1.0f},5.0f, 0.001f);
    SDL_SetWindowRelativeMouseMode(get_window(), false);

	/* Initialize global light direction */
	initialize_global_light();

    set_shading_method(SHADING_GOURAUD);
	set_render_method(RENDER_TEXTURED);

    return true;
}

void initialize_global_light(void)
{
	set_global_light_direction((brh_vector3) { 0.0f, 0.0f, 1.0f });
}

/* Helper to load mesh and related resources */
bool load_mesh_resources(void)
{
    // Initialize all renderable slots to NULL
    for (int i = 0; i < MAX_NUM_RENDERABLES; i++) {
        renderables[i] = NULL;
    }

    // Create our renderables
    f117_renderable = create_renderable_from_files("assets/f117.obj", "assets/f117.png");
    if (!f117_renderable) {
        fprintf(stderr, "Error: Failed to create F117 renderable\n");
        return false;
    }
    renderables[0] = f117_renderable;

    f22_renderable = create_renderable_from_files("assets/f22.obj", "assets/f22.png");
    if (!f22_renderable) {
        fprintf(stderr, "Error: Failed to create F22 renderable\n");
        destroy_renderable(f117_renderable);
        return false;
    }
    renderables[1] = f22_renderable;

    mirage_renderable = create_renderable_from_files("assets/efa.obj", "assets/efa.png");
    if (!mirage_renderable) {
        fprintf(stderr, "Error: Failed to create Mirage renderable\n");
        destroy_renderable(f117_renderable);
        destroy_renderable(f22_renderable);
        return false;
    }
    renderables[2] = mirage_renderable;

	crab_renderable = create_renderable_from_files("assets/crab.obj", "assets/crab.png");
	if (!crab_renderable) {
		fprintf(stderr, "Error: Failed to create Crab renderable\n");
		destroy_renderable(f117_renderable);
		destroy_renderable(f22_renderable);
		destroy_renderable(mirage_renderable);
		return false;
	}
	renderables[3] = crab_renderable;

	drone_renderable = create_renderable_from_files("assets/drone.obj", "assets/drone.png");
	if (!drone_renderable) {
		fprintf(stderr, "Error: Failed to create Drone renderable\n");
		destroy_renderable(f117_renderable);
		destroy_renderable(f22_renderable);
		destroy_renderable(mirage_renderable);
		destroy_renderable(crab_renderable);
		return false;
	}
	renderables[4] = drone_renderable;

    // Set initial positions
    set_renderable_position(f117_renderable, (brh_vector3) { -5.0f, 0.0f, 5.0f });
    set_renderable_position(f22_renderable, (brh_vector3) { 0.0f, 0.0f, 5.0f });
    set_renderable_position(mirage_renderable, (brh_vector3) { 5.0f, 0.0f, 5.0f });
	set_renderable_position(crab_renderable, (brh_vector3) { 0.0f, 0.0f, 10.0f });
	set_renderable_position(drone_renderable, (brh_vector3) { 0.0f, 0.0f, 15.0f });

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
                mouse_locked = !mouse_locked;
                SDL_SetWindowRelativeMouseMode(get_window(), mouse_locked);
                // Reset mouse init flag when locking/unlocking
                mouse_initialized = false;
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (mouse_locked) {
                // Only process mouse movement if mouse is locked
                int mouse_x_rel = (int)event.motion.xrel;
                int mouse_y_rel = (int)event.motion.yrel;

                // Skip the first motion event after locking to avoid jumps
                if (!mouse_initialized) {
                    mouse_initialized = true;
                }
                else {
                    update_mouse_camera_view(mouse_camera, mouse_x_rel, mouse_y_rel);
                }
            }
            break;

        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key)
            {
            case SDLK_ESCAPE: is_running = false; break;
                // Render Method Keys
            case SDLK_1: set_render_method(RENDER_WIREFRAME_VERTEX); break;
            case SDLK_2: set_render_method(RENDER_WIREFRAME); break;
            case SDLK_3: set_render_method(RENDER_FILL); break;
            case SDLK_4: set_render_method(RENDER_FILL_WIREFRAME); break;
            case SDLK_5: set_render_method(RENDER_TEXTURED); break;
            case SDLK_6: set_render_method(RENDER_TEXTURED_WIREFRAME); break;
                // Culling Keys
            case SDLK_C: set_cull_method(CULL_BACKFACE); break;
            case SDLK_X: set_cull_method(CULL_NONE); break;
                // Shading Keys
            case SDLK_F1: set_shading_method(SHADING_NONE); printf("Shading: None\n"); break;
            case SDLK_F2: set_shading_method(SHADING_FLAT); printf("Shading: Flat\n"); break;
            case SDLK_F3: set_shading_method(SHADING_GOURAUD); printf("Shading: Gouraud\n"); break;
            case SDLK_F4: set_shading_method(SHADING_PHONG); printf("Shading: Phong\n"); break; // Add Phong key
                // Camera movement controls
            case SDLK_W: movement_forward = 1; break;
            case SDLK_S: movement_forward = -1; break;
            case SDLK_D: movement_right = 1; break;
            case SDLK_A: movement_right = -1; break;
            case SDLK_SPACE: movement_up = 1; break;
            case SDLK_LCTRL: movement_up = -1; break;
            }
            break;
        case SDL_EVENT_KEY_UP:
            switch (event.key.key)
            {
            case SDLK_W: if (movement_forward == 1) movement_forward = 0; break;
            case SDLK_S: if (movement_forward == -1) movement_forward = 0; break;
            case SDLK_A: if (movement_right == -1) movement_right = 0; break;
            case SDLK_D: if (movement_right == 1) movement_right = 0; break;
            case SDLK_SPACE: if (movement_up == 1) movement_up = 0; break;
            case SDLK_LCTRL: if (movement_up == -1) movement_up = 0; break;
            }
            break;
        }
    }

    // Update camera position based on movement keys (only if locked)
    if (mouse_locked) {
        move_mouse_camera(mouse_camera, movement_forward, movement_right, movement_up, delta_time_seconds);
    }
}

/* --------- Update Game State --------- */
void update(void)
{
    /* Frame timing management (unchanged) */
    uint32_t time_to_wait = FRAME_TARGET_TIME - ((uint32_t)SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    delta_time_seconds = (SDL_GetTicks() - previous_frame_time) / 1000.0f;
    previous_frame_time = (uint32_t)SDL_GetTicks();

    /* Get the world matrix from the renderable */
    camera_matrix = get_mouse_camera_view_matrix(mouse_camera);

    /* Process mesh faces (unchanged, but using mesh_data) */
    update_renderables(delta_time_seconds, camera_matrix, perspective_projection_matrix, mouse_camera); 
}

/* --------- Render the Scene --------- */
void render(void)
{
    const enum render_method current_render_method = get_render_method();
    // Shading method is implicitly retrieved by the functions called below

    /* Clear buffers */
    clear_color_buffer(0xFF101020);  // Dark blue/purple background
    clear_z_buffer();

    /* Draw background grid (optional) */
    // draw_grid(cell_size, 0xFF333333);

    /* Render each renderable */
    for (int r = 0; r < MAX_NUM_RENDERABLES; r++) {
        if (renderables[r] == NULL) continue; // Skip invalid/unloaded renderables

        brh_texture_handle texture = get_renderable_texture(renderables[r]); // Get texture handle
        brh_triangle* triangles = get_renderable_triangles(renderables[r]);
        int triangle_count = get_renderable_triangle_count(renderables[r]);

        // Render all triangles for this renderable
        for (int i = 0; i < triangle_count; i++) {
            brh_triangle* triangle = &triangles[i]; // Get pointer to the triangle

            // Draw filled/textured triangles first (they use Z-buffer)
            bool needs_fill = (current_render_method == RENDER_FILL || current_render_method == RENDER_FILL_WIREFRAME);
            bool needs_texture = (current_render_method == RENDER_TEXTURED || current_render_method == RENDER_TEXTURED_WIREFRAME);

            if (needs_texture && texture != NULL) {
                // Pass the texture handle to draw_textured_triangle
                draw_textured_triangle(triangle, texture);
            }
            else if (needs_fill || (needs_texture && texture == NULL)) { // Fallback to fill if texture needed but missing
                // Pass the triangle's base color (or flat-shaded color if applicable)
                draw_filled_triangle(triangle, triangle->color);
            }

            // Draw wireframe overlay if required (drawn on top)
            if (current_render_method == RENDER_WIREFRAME ||
                current_render_method == RENDER_WIREFRAME_VERTEX ||
                current_render_method == RENDER_FILL_WIREFRAME ||
                current_render_method == RENDER_TEXTURED_WIREFRAME) {
                draw_triangle_outline(triangle, 0xFFBBBBBB);  // Light gray wireframe
            }

            // Draw vertex markers if required (drawn on top)
            if (current_render_method == RENDER_WIREFRAME_VERTEX) {
                for (int j = 0; j < 3; j++) {
                    draw_rect(
                        (int)triangle->vertices[j].position.x - 2, // Smaller rect
                        (int)triangle->vertices[j].position.y - 2,
                        4, 4, 0xFFFF0000 // Red vertices
                    );
                }
            }
        } // End triangle loop
    } // End renderable loop

    /* Present the frame */
    render_color_buffer();
}

/* --------- Mesh Resource Cleanup --------- */
void cleanup_mesh_resources(void)
{
    /* Destroy the renderable */
    if (f117_renderable) {
        destroy_renderable(f117_renderable);
        f117_renderable = NULL;
    }
}

void cleanup_camera_resources(void)
{
	destroy_look_at_camera(lookat_camera);
	destroy_mouse_camera(mouse_camera);
}

/* --------- Resource Cleanup --------- */
void cleanup_resources(void)
{
    // Free mesh resources
    cleanup_mesh_resources();
    cleanup_camera_resources();

    // Clean up display resources
    cleanup_display_resources();
}