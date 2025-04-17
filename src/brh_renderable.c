#include <stdlib.h>
#include <stdio.h>
#include "brh_renderable.h"
#include "brh_mesh_manager.h"
#include "brh_texture_manager.h"
#include "array.h"
#include "brh_light.h"
#include "brh_matrix.h"
#include "brh_clipping.h"
#include "brh_display.h"
#include "math_utils.h"

#define MAX_RENDERABLES 32  // Maximum number of renderables that can be created simultaneously

static void update_renderable_triangles(brh_renderable_handle renderable_handle, brh_mat4 camera_matrix, brh_mat4 projection_matrix, brh_vector3 camera_pos_world);

typedef struct brh_renderable_handle_t {
    int id;                  // Unique identifier for this renderable
    brh_mesh_handle mesh;    // Handle to the mesh
    brh_texture_handle texture; // Handle to the texture (can be NULL)
    brh_vector3 position;    // Position in world space
    brh_vector3 rotation;    // Rotation in world space (in radians)
    brh_vector3 scale;       // Scale in world space
    brh_mat4 world_matrix;   // Cached world matrix
    // Rendering data
    brh_triangle* triangles;      // Buffer of triangles to render
    int triangle_count;           // Number of triangles in the buffer
    int triangle_capacity;        // Capacity of the triangle buffer
    bool is_valid;           // Whether this handle is valid
    bool needs_update;       // Whether the world matrix needs to be recalculated
    bool owns_resources;     // Whether this renderable owns its mesh and texture
} brh_renderable_handle_t;

// Array of renderable handles
static brh_renderable_handle_t renderable_handles[MAX_RENDERABLES];
static int next_renderable_id = 1;  // Start from 1, 0 can be reserved for invalid handles

bool initialize_renderable_system(void)
{
    // Initialize all renderable handles as invalid
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        renderable_handles[i].id = 0;
        renderable_handles[i].mesh = NULL;
        renderable_handles[i].texture = NULL;
        renderable_handles[i].position = (brh_vector3){ 0.0f, 0.0f, 0.0f };
        renderable_handles[i].rotation = (brh_vector3){ 0.0f, 0.0f, 0.0f };
        renderable_handles[i].scale = (brh_vector3){ 1.0f, 1.0f, 1.0f };
        renderable_handles[i].world_matrix = mat4_identity();
        renderable_handles[i].is_valid = false;
        renderable_handles[i].needs_update = true;
        renderable_handles[i].owns_resources = false;
    }
    next_renderable_id = 1;
    return true;
}

void cleanup_renderable_system(void)
{
    // Free all valid renderables
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        if (renderable_handles[i].is_valid) {
            destroy_renderable((brh_renderable_handle)&renderable_handles[i]);
        }
    }
}

brh_renderable_handle create_renderable(brh_mesh_handle mesh_handle, brh_texture_handle texture_handle)
{
    // Check if mesh handle is valid
    if (mesh_handle && get_mesh_data(mesh_handle) == NULL) {
        fprintf(stderr, "Error: Invalid mesh handle\n");
        return NULL;
    }

    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        if (!renderable_handles[i].is_valid) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "Error: Maximum number of renderables (%d) reached\n", MAX_RENDERABLES);
        return NULL;
    }

    // Get mesh face count to allocate triangle buffer
    int face_count = 0;
    if (mesh_handle) {
        brh_mesh* mesh_data = get_mesh_data(mesh_handle);
        face_count = array_length(mesh_data->faces);
    }

    // Allocate triangle buffer
    brh_triangle* triangles = NULL;
    if (face_count > 0) {
        triangles = (brh_triangle*)malloc(sizeof(brh_triangle) * face_count);
        if (!triangles) {
            fprintf(stderr, "Error: Failed to allocate triangle buffer for renderable\n");
            return NULL;
        }
    }

    // Setup the handle
    renderable_handles[slot].id = next_renderable_id++;
    renderable_handles[slot].mesh = mesh_handle;
    renderable_handles[slot].texture = texture_handle;
    renderable_handles[slot].position = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    renderable_handles[slot].rotation = (brh_vector3){ 0.0f, 0.0f, 0.0f };
    renderable_handles[slot].scale = (brh_vector3){ 1.0f, 1.0f, 1.0f };
    renderable_handles[slot].world_matrix = mat4_identity();
    renderable_handles[slot].triangles = triangles;
    renderable_handles[slot].triangle_count = 0;
    renderable_handles[slot].triangle_capacity = face_count;
    renderable_handles[slot].is_valid = true;
    renderable_handles[slot].needs_update = true;
    renderable_handles[slot].owns_resources = false;

    return (brh_renderable_handle)&renderable_handles[slot];
}

brh_renderable_handle create_renderable_from_files(const char* mesh_file, const char* texture_file)
{
    // Load mesh
    brh_mesh_handle mesh_handle = load_mesh(mesh_file, true);
    if (!mesh_handle) {
        fprintf(stderr, "Error: Failed to load mesh: %s\n", mesh_file);
        return NULL;
    }

    // Load texture if provided
    brh_texture_handle texture_handle = NULL;
    if (texture_file != NULL) {
        texture_handle = load_texture(texture_file);
        if (!texture_handle) {
            fprintf(stderr, "Error: Failed to load texture: %s\n", texture_file);
            unload_mesh(mesh_handle);
            return NULL;
        }
    }

    // Create renderable
    brh_renderable_handle renderable = create_renderable(mesh_handle, texture_handle);
    if (!renderable) {
        if (texture_handle) unload_texture(texture_handle);
        unload_mesh(mesh_handle);
        return NULL;
    }

    // Mark that this renderable owns its resources
    ((brh_renderable_handle_t*)renderable)->owns_resources = true;

    return renderable;
}

void destroy_renderable(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;

    // Free triangle buffer
    if (handle->triangles) {
        free(handle->triangles);
        handle->triangles = NULL;
        handle->triangle_count = 0;
        handle->triangle_capacity = 0;
    }

    // If this renderable owns its resources, unload them
    if (handle->owns_resources) {
        if (handle->texture) {
            unload_texture(handle->texture);
        }
        if (handle->mesh) {
            unload_mesh(handle->mesh);
        }
    }

    // Invalidate the handle
    handle->is_valid = false;
    handle->mesh = NULL;
    handle->texture = NULL;
}

void set_renderable_position(brh_renderable_handle renderable_handle, brh_vector3 position)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;
    handle->position = position;
    handle->needs_update = true;
}

void set_renderable_rotation(brh_renderable_handle renderable_handle, brh_vector3 rotation)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;
    handle->rotation = rotation;
    handle->needs_update = true;
}

void set_renderable_scale(brh_renderable_handle renderable_handle, brh_vector3 scale)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;
    handle->scale = scale;
    handle->needs_update = true;
}

brh_vector3 get_renderable_position(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    return ((brh_renderable_handle_t*)renderable_handle)->position;
}

brh_vector3 get_renderable_rotation(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return (brh_vector3) { 0.0f, 0.0f, 0.0f };
    }

    return ((brh_renderable_handle_t*)renderable_handle)->rotation;
}

brh_vector3 get_renderable_scale(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return (brh_vector3) { 1.0f, 1.0f, 1.0f };
    }

    return ((brh_renderable_handle_t*)renderable_handle)->scale;
}

brh_mat4 get_renderable_world_matrix(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return mat4_identity();
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;

    // Update world matrix if needed
    if (handle->needs_update) {
        handle->world_matrix = mat4_create_world_matrix(
            handle->position,
            handle->rotation,
            handle->scale
        );
        handle->needs_update = false;
    }

    return handle->world_matrix;
}

brh_mesh_handle get_renderable_mesh(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return NULL;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->mesh;
}

brh_texture_handle get_renderable_texture(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return NULL;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->texture;
}

static void update_renderable_triangles(brh_renderable_handle renderable_handle, brh_mat4 camera_matrix, brh_mat4 projection_matrix, brh_vector3 camera_pos_world)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return;
    }

    brh_renderable_handle_t* handle = (brh_renderable_handle_t*)renderable_handle;

    // Skip if no mesh or no triangle buffer
    if (!handle->mesh || !handle->triangles || handle->triangle_capacity <= 0) {
        handle->triangle_count = 0; // Ensure count is zero if no mesh/buffer
        return;
    }

    // Get mesh data
    brh_mesh* mesh_data = get_mesh_data(handle->mesh);
    if (!mesh_data || !mesh_data->vertices || !mesh_data->faces) {
        handle->triangle_count = 0; // Ensure count is zero if mesh data invalid
        return;
    }

    // Reset triangle count for this frame
    handle->triangle_count = 0;

    // Get world matrix and calculate normal matrix (inverse transpose of upper 3x3)
    brh_mat4 world_matrix = get_renderable_world_matrix(renderable_handle);
    // For simplicity, if only uniform scale/rotation/translation, just use upper 3x3 of world matrix for normal transform.
    // A proper implementation uses inverse transpose. Let's approximate for now.
    // TODO: Implement proper inverse transpose for normal transformation if non-uniform scaling is used.
    brh_mat4 normal_matrix = world_matrix; // Approximation!
    normal_matrix.m[3][0] = normal_matrix.m[3][1] = normal_matrix.m[3][2] = 0.0f; // Zero out translation

    int num_faces = array_length(mesh_data->faces);
    int num_vertices = array_length(mesh_data->vertices);
    int num_texcoords = array_length(mesh_data->texcoords);
    int num_normals = array_length(mesh_data->normals);

    // Get current shading method
    shading_method current_shading = get_shading_method();

    // Temporary buffer for clipped triangles
    brh_triangle clipped_triangles[MAX_CLIPPED_TRIANGLES]; // Defined in brh_clipping.h

    for (int i = 0; i < num_faces && handle->triangle_count < handle->triangle_capacity; i++) {
        brh_face face = mesh_data->faces[i];

        // Basic validation of face indices
        if (face.a < 0 || face.a >= num_vertices ||
            face.b < 0 || face.b >= num_vertices ||
            face.c < 0 || face.c >= num_vertices) {
            fprintf(stderr, "Warning: Invalid vertex index in face %d\n", i);
            continue;
        }

        brh_vertex triangle_vertices[3]; // Holds processed vertex data for the triangle
        brh_vector3 face_vertices_model[3];
        brh_vector3 face_normals_model[3] = { {0,0,1}, {0,0,1}, {0,0,1} }; // Default normal
        brh_texel face_texels[3] = { {0, 0}, {0, 0}, {0, 0} };
        brh_vector4 face_vertices_world[3];
        brh_vector4 face_vertices_camera[3];
        brh_vector3 face_normals_world[3];

        // --- 1. Gather Vertex Data (Position, Texcoord, Normal) ---
        face_vertices_model[0] = mesh_data->vertices[face.a];
        face_vertices_model[1] = mesh_data->vertices[face.b];
        face_vertices_model[2] = mesh_data->vertices[face.c];

        if (num_texcoords > 0) {
            // Validate texcoord indices before access
            if (face.a_vt >= 0 && face.a_vt < num_texcoords) face_texels[0] = mesh_data->texcoords[face.a_vt];
            if (face.b_vt >= 0 && face.b_vt < num_texcoords) face_texels[1] = mesh_data->texcoords[face.b_vt];
            if (face.c_vt >= 0 && face.c_vt < num_texcoords) face_texels[2] = mesh_data->texcoords[face.c_vt];
        }

        if (num_normals > 0) {
            // Validate normal indices before access
            if (face.a_vn >= 0 && face.a_vn < num_normals) face_normals_model[0] = mesh_data->normals[face.a_vn];
            if (face.b_vn >= 0 && face.b_vn < num_normals) face_normals_model[1] = mesh_data->normals[face.b_vn];
            if (face.c_vn >= 0 && face.c_vn < num_normals) face_normals_model[2] = mesh_data->normals[face.c_vn];
        }

        // --- 2. Transform Vertices and Normals, Calculate Initial Vertex Data ---
        for (int j = 0; j < 3; j++) {
            // Transform vertex position to World -> Camera -> Clip space
            brh_vector4 world_vertex = vec4_from_vec3(face_vertices_model[j]);
            mat4_mul_vec4_ref(&world_matrix, &world_vertex);
            face_vertices_world[j] = world_vertex; // Store world pos (needed for lighting)

            brh_vector4 camera_vertex = world_vertex;
            mat4_mul_vec4_ref(&camera_matrix, &camera_vertex);
            face_vertices_camera[j] = camera_vertex; // Store camera pos (needed for culling)

            brh_vector4 clip_vertex = mat4_mul_vec4(&projection_matrix, camera_vertex);

            // Transform vertex normal to World Space (using approximation)
            brh_vector4 normal = vec4_from_vec3(face_normals_model[j]);
            normal.w = 0; // Normals are directions, ignore translation
            mat4_mul_vec4_ref(&normal_matrix, &normal);
            face_normals_world[j] = vec3_unit_vector(vec3_from_vec4(normal)); // Normalize world normal

            // Store initial data in brh_vertex
            triangle_vertices[j].position = clip_vertex; // Clip space position
            triangle_vertices[j].texel = face_texels[j];
            triangle_vertices[j].normal = face_normals_world[j]; // Store WORLD SPACE normal
            triangle_vertices[j].color = face.color; // Store base color temporarily

            // Calculate 1/w for perspective correction (handle w=0 case)
            if (fabsf(clip_vertex.w) < EPSILON) {
                triangle_vertices[j].inv_w = 0.0f; // Avoid division by zero
            }
            else {
                triangle_vertices[j].inv_w = 1.0f / clip_vertex.w;
            }
        }

        // --- 3. Backface Culling (in Camera Space) ---
        if (get_cull_method() == CULL_BACKFACE) {
            brh_vector3 vecA_camera = vec3_from_vec4(face_vertices_camera[0]);
            brh_vector3 vecB_camera = vec3_from_vec4(face_vertices_camera[1]);
            brh_vector3 vecC_camera = vec3_from_vec4(face_vertices_camera[2]);

            // Calculate normal in camera space for culling
            brh_vector3 normal_camera = get_face_normal(vecA_camera, vecB_camera, vecC_camera);

            // View vector from origin (camera) to vertex A in camera space
            brh_vector3 view_vector_camera = vec3_from_vec4(face_vertices_camera[0]); // Or just vecA_camera

            // Dot product determines if face is visible
            float angle_dot_product = vec3_dot(normal_camera, view_vector_camera);

            // Cull if angle is >= 90 degrees (normal points away or parallel to view)
            if (angle_dot_product >= 0) {
                continue; // Skip this face
            }
        }

        // --- 4. Calculate Shading (based on method) ---
        uint32_t flat_shaded_color = face.color; // Used if flat shading

        if (current_shading == SHADING_FLAT) {
            // Calculate flat shading using the geometric normal in world space
            brh_vector3 world_v0 = vec3_from_vec4(face_vertices_world[0]);
            brh_vector3 world_v1 = vec3_from_vec4(face_vertices_world[1]);
            brh_vector3 world_v2 = vec3_from_vec4(face_vertices_world[2]);
            brh_vector3 face_normal_world = get_face_normal(world_v0, world_v1, world_v2);
            flat_shaded_color = calculate_flat_shading_color(face_normal_world, face.color);
            // Store this color in the vertices (will be constant across the clipped triangle)
            triangle_vertices[0].color = flat_shaded_color;
            triangle_vertices[1].color = flat_shaded_color;
            triangle_vertices[2].color = flat_shaded_color;
        }
        else if (current_shading == SHADING_GOURAUD) {
            // Calculate lighting per vertex and store in vertex.color
            for (int j = 0; j < 3; j++) {
                brh_vector3 vertex_pos_world = vec3_from_vec4(face_vertices_world[j]);
                triangle_vertices[j].color = calculate_vertex_shading_color(
                    triangle_vertices[j].normal, // Already calculated world-space normal
                    vertex_pos_world,
                    camera_pos_world,
                    face.color // Base color for the vertex
                );
            }
        }
        // For SHADING_PHONG and SHADING_NONE, we don't pre-calculate colors here.
        // Phong needs interpolated normals, None uses base color/texture directly.
        // The world-space normals are already stored in triangle_vertices[j].normal


        // --- 5. Assemble Triangle for Clipping ---
        brh_triangle clip_space_triangle = {
            .vertices = { triangle_vertices[0], triangle_vertices[1], triangle_vertices[2] },
            .color = (current_shading == SHADING_FLAT) ? flat_shaded_color : face.color, // Pass flat color or original face color
        };


        // --- 6. Clip Triangle ---
        int num_clipped_triangles = clip_triangle(&clip_space_triangle, clipped_triangles);

        // --- 7. Process Clipped Triangles ---
        for (int k = 0; k < num_clipped_triangles && handle->triangle_count < handle->triangle_capacity; k++) {
            brh_triangle triangle_to_render = clipped_triangles[k];

            // --- 8. Perspective Division & Viewport Transformation ---
            for (int v = 0; v < 3; v++) {
                brh_vector4 clip_pos = triangle_to_render.vertices[v].position;

                // Perspective division (guard against w near zero)
                float inv_w = (fabsf(clip_pos.w) < EPSILON) ? 0.0f : (1.0f / clip_pos.w);
                brh_vector4 ndc_vertex; // Normalized Device Coordinates
                ndc_vertex.x = clip_pos.x * inv_w;
                ndc_vertex.y = clip_pos.y * inv_w;
                ndc_vertex.z = clip_pos.z * inv_w; // Typically used for depth buffer (0 to 1 or -1 to 1)
                // We will store 1/w in the Z-buffer for depth comparison later
                // ndc_vertex.w = clip_pos.w; // Not needed after division

                // Viewport transform
                // Map NDC X/Y from [-1, 1] to screen coordinates [0, Width]/[0, Height]
                // Note: Y is often flipped (NDC +1 is top, screen +1 is bottom)
                triangle_to_render.vertices[v].position.x = (ndc_vertex.x + 1.0f) * 0.5f * (float)get_window_width();
                triangle_to_render.vertices[v].position.y = (1.0f - ndc_vertex.y) * 0.5f * (float)get_window_height(); // Flip Y

                // Store 1/w for depth testing and perspective correction during rasterization
                // We use the *original* clip-space W for calculating 1/w
                triangle_to_render.vertices[v].inv_w = inv_w;

                // Keep Z and W from clip space if needed later (though inv_w is primary for depth/interpolation)
                triangle_to_render.vertices[v].position.z = clip_pos.z; // Keep original Z if needed
                triangle_to_render.vertices[v].position.w = clip_pos.w; // Keep original W if needed
            }

            // Add the final screen-space triangle to the renderable's buffer
            handle->triangles[handle->triangle_count++] = triangle_to_render;
        }


        if (handle->triangle_count >= handle->triangle_capacity) {
            fprintf(stderr, "Warning: Renderable %d triangle buffer filled to capacity (%d triangles)\n", handle->id, handle->triangle_capacity);
            break; // Stop processing faces for this renderable if buffer is full
        }
    } // End face loop
}

brh_triangle* get_renderable_triangles(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return NULL;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->triangles;
}

int get_renderable_triangle_count(brh_renderable_handle renderable_handle)
{
    if (!renderable_handle || !((brh_renderable_handle_t*)renderable_handle)->is_valid) {
        return 0;
    }

    return ((brh_renderable_handle_t*)renderable_handle)->triangle_count;
}

void update_renderables(float delta_time, brh_mat4 camera_matrix, brh_mat4 projection_matrix, brh_mouse_camera* camera)
{
    // Update all valid renderables
    for (int i = 0; i < MAX_RENDERABLES; i++) {
        if (renderable_handles[i].is_valid) {
            // Update world matrix if needed
            if (renderable_handles[i].needs_update) {
                renderable_handles[i].world_matrix = mat4_create_world_matrix(
                    renderable_handles[i].position,
                    renderable_handles[i].rotation,
                    renderable_handles[i].scale
                );
                renderable_handles[i].needs_update = false;
            }

            // Update triangles
            update_renderable_triangles((brh_renderable_handle)&renderable_handles[i],
                camera_matrix,
                projection_matrix,
                get_mouse_camera_position(camera));
        }
    }
}