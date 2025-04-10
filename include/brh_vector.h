#pragma once

// ============================================================================
// Vector2 Structure and Functions
// ============================================================================

/**
 * @brief A structure representing a 2D vector.
 */
typedef struct {
    float x;
    float y;
} brh_vector2;

/**
 * @brief A structure representing a 3D vector.
 */
typedef struct {
    float x;
    float y;
    float z;
} brh_vector3;

/**
 * @brief A structure representing a 4D vector.
 */ 
typedef struct {
    float x;
    float y;
    float z;
    float w;
} brh_vector4;

/**
 * @brief Creates a `brh_vector2` instance with the specified x and y components.
 *
 * @param x The x component of the vector.
 * @param y The y component of the vector.
 *
 * @return A `brh_vector2` instance with the specified components.
 */
brh_vector2 vec2_create(float x, float y);

/**
 * @brief Calculate the magnitude of a 2D vector.
 * @param v The vector.
 * @return The magnitude of the vector.
 */
float vec2_magnitude(brh_vector2 v);

/**
 * @brief Add two 2D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The result of the addition.
 */
brh_vector2 vec2_add(brh_vector2 a, brh_vector2 b);

/**
 * @brief Subtract one 2D vector from another.
 * @param a The first vector.
 * @param b The second vector.
 * @return The result of the subtraction.
 */
brh_vector2 vec2_subtract(brh_vector2 a, brh_vector2 b);

/**
 * @brief Scale a 2D vector by a scalar.
 * @param v The vector.
 * @param scalar The scalar value.
 * @return The scaled vector.
 */
brh_vector2 vec2_scale(brh_vector2 v, float scalar);

/**
 * @brief Calculate the normal of a 2D vector.
 * @param v The vector.
 * @return The normal vector.
 */
brh_vector2 vec2_normal(brh_vector2 v);

/**
 * @brief Normalize a 2D vector.
 * @param v A pointer to the vector to normalize.
 */
void vec2_normalize(brh_vector2* v);

/**
 * @brief Calculate the dot product of two 2D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The dot product.
 */
float vec2_dot(brh_vector2 a, brh_vector2 b);

/**
 * @brief Calculate the cross product of two 2D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The cross product.
 */
float vec2_cross(brh_vector2 a, brh_vector2 b);

/**
 * @brief Calculate the angle between two 2D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The angle in radians.
 */
float vec2_angle(brh_vector2 a, brh_vector2 b);

/**
 * @brief Add a 2D vector to another by reference.
 * @param a A pointer to the first vector.
 * @param b The second vector.
 */
void vec2_add_ref(brh_vector2* a, brh_vector2 b);

/**
 * @brief Subtract a 2D vector from another by reference.
 * @param a A pointer to the first vector.
 * @param b The second vector.
 */
void vec2_subtract_ref(brh_vector2* a, brh_vector2 b);

/**
 * @brief Scale a 2D vector by a scalar by reference.
 * @param v A pointer to the vector.
 * @param scalar The scalar value.
 */
void vec2_scale_ref(brh_vector2* v, float scalar);

/**
 * @brief Rotate a 2D vector by an angle.
 * @param v The vector.
 * @param angle The angle in radians.
 * @return The rotated vector.
 */
brh_vector2 vec2_rotate(brh_vector2 v, float angle);

/**
 * @brief Rotate a 2D vector by an angle by reference.
 * @param v A pointer to the vector.
 * @param angle The angle in radians.
 */
brh_vector2 vec2_rotate_ref(brh_vector2* v, float angle);

// ============================================================================
// Vector3 Structure and Functions
// ============================================================================

/*
* @brief Creates a `brh_vector3` instance with the specified x, y, and z components.
*
* @param x The x component of the vector.
* @param y The y component of the vector.
* @param z The z component of the vector.
*
* @return A `brh_vector3` instance with the specified components.
*/
brh_vector3 vec3_create(float x, float y, float z);

/**
 * @brief Calculate the magnitude of a 3D vector.
 * @param v The vector.
 * @return The magnitude of the vector.
 */
float vec3_magnitude(brh_vector3 v);

/**
 * @brief Add two 3D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The result of the addition.
 */
brh_vector3 vec3_add(brh_vector3 a, brh_vector3 b);

/**
 * @brief Subtract one 3D vector from another.
 * @param a The first vector.
 * @param b The second vector.
 * @return The result of the subtraction.
 */
brh_vector3 vec3_subtract(brh_vector3 a, brh_vector3 b);

/**
 * @brief Scale a 3D vector by a scalar.
 * @param v The vector.
 * @param scalar The scalar value.
 * @return The scaled vector.
 */
brh_vector3 vec3_scale(brh_vector3 v, float scalar);

/**
 * @brief Converts the 3D vector to a unit vector.
 * @param v The vector.
 * @return The normalized vector.
 */
brh_vector3 vec3_unit_vector(brh_vector3 v);

/**
 * @brief Normalize a 3D vector.
 * @param v A pointer to the vector to normalize.
 */
void vec3_normalize(brh_vector3* v);

/**
 * @brief Add a 3D vector to another by reference.
 * @param a A pointer to the first vector.
 * @param b The second vector.
 */
void vec3_add_ref(brh_vector3* a, brh_vector3 b);

/**
 * @brief Subtract a 3D vector from another by reference.
 * @param a A pointer to the first vector.
 * @param b The second vector.
 */
void vec3_subtract_ref(brh_vector3* a, brh_vector3 b);

/**
 * @brief Scale a 3D vector by a scalar by reference.
 * @param v A pointer to the vector.
 * @param scalar The scalar value.
 */
void vec3_scale_ref(brh_vector3* v, float scalar);

/**
 * @brief Calculate the dot product of two 3D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The dot product.
 */
float vec3_dot(brh_vector3 a, brh_vector3 b);

/**
 * @brief Calculate the cross product of two 3D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The cross product.
 */
brh_vector3 vec3_cross(brh_vector3 a, brh_vector3 b);

/**
 * @brief Calculate the angle between two 3D vectors.
 * @param a The first vector.
 * @param b The second vector.
 * @return The angle in radians.
 */
float vec3_angle(brh_vector3 a, brh_vector3 b);

/**
 * @brief Rotate a 3D vector around the X-axis.
 * @param v The vector.
 * @param angle The angle in radians.
 * @return The rotated vector.
 */
brh_vector3 vec3_rotate_x(brh_vector3 v, float angle);

/**
 * @brief Rotate a 3D vector around the Y-axis.
 * @param v The vector.
 * @param angle The angle in radians.
 * @return The rotated vector.
 */
brh_vector3 vec3_rotate_y(brh_vector3 v, float angle);

/**
 * @brief Rotate a 3D vector around the Z-axis.
 * @param v The vector.
 * @param angle The angle in radians.
 * @return The rotated vector.
 */
brh_vector3 vec3_rotate_z(brh_vector3 v, float angle);

/**
 * @brief Rotate a 3D vector around the X-axis by reference.
 * @param v A pointer to the vector.
 * @param angle The angle in radians.
 */
brh_vector3 vec3_rotate_x_ref(brh_vector3* v, float angle);

/**
 * @brief Rotate a 3D vector around the Y-axis by reference.
 * @param v A pointer to the vector.
 * @param angle The angle in radians.
 */
brh_vector3 vec3_rotate_y_ref(brh_vector3* v, float angle);

/**
 * @brief Rotate a 3D vector around the Z-axis by reference.
 * @param v A pointer to the vector.
 * @param angle The angle in radians.
 */
brh_vector3 vec3_rotate_z_ref(brh_vector3* v, float angle);

/**
 * @brief Convert a 4D vector to a 3D vector.
 * @param v The 4D vector.
 * @return The converted 3D vector.
 */
brh_vector3 vec3_from_vec4(brh_vector4 v);

// ============================================================================
// Vector4 Functions
// ============================================================================

/**
 * @brief Convert a 3D vector to a 4D vector.
 * @param v The 3D vector.
 * @return The converted 4D vector.
 */
brh_vector4 vec4_from_vec3(brh_vector3 v);
