#pragma once

#include "brh_vector.h"

/**
 * @brief A structure representing a 4x4 matrix.
 */
typedef struct {
    float m[4][4];
} brh_mat4;

/**
 * @brief Creates an identity matrix.
 * 
 * An identity matrix is a square matrix with ones on the main diagonal and zeros elsewhere.
 * 
 * @return A 4x4 identity matrix.
 * 
 * @code
 * 1 0 0 0
 * 0 1 0 0
 * 0 0 1 0
 * 0 0 0 1
 * @endcode
 */
brh_mat4 mat4_identity(void);

/**
 * @brief Creates a scaling matrix.
 * 
 * A scaling matrix scales the coordinates of a vector.
 * 
 * @param sx Scaling factor along the x-axis.
 * @param sy Scaling factor along the y-axis.
 * @param sz Scaling factor along the z-axis.
 * @return A 4x4 scaling matrix.
 * 
 * @code
 * sx  0  0  0
 *  0 sy  0  0
 *  0  0 sz  0
 *  0  0  0  1
 * @endcode
 */
brh_mat4 mat4_create_scale(float sx, float sy, float sz);

/**
 * @brief Creates a translation matrix.
 * 
 * A translation matrix translates the coordinates of a vector.
 * 
 * @param tx Translation along the x-axis.
 * @param ty Translation along the y-axis.
 * @param tz Translation along the z-axis.
 * @return A 4x4 translation matrix.
 * 
 * @code
 * 1  0  0 tx
 * 0  1  0 ty
 * 0  0  1 tz
 * 0  0  0  1
 * @endcode
 */
brh_mat4 mat4_create_translation(float tx, float ty, float tz);

/**
 * @brief Creates a rotation matrix around the x-axis.
 * 
 * A rotation matrix rotates the coordinates of a vector around the x-axis.
 * 
 * @param theta Angle of rotation in radians.
 * @return A 4x4 rotation matrix.
 * 
 * @code
 * 1  0   0  0
 * 0  cos sin 0
 * 0 -sin cos 0
 * 0  0   0  1
 * @endcode
 */
brh_mat4 mat4_create_rotation_x(float theta);

/**
 * @brief Creates a rotation matrix around the y-axis.
 * 
 * A rotation matrix rotates the coordinates of a vector around the y-axis.
 * 
 * @param theta Angle of rotation in radians.
 * @return A 4x4 rotation matrix.
 * 
 * @code
 * cos  0 -sin 0
 *  0   1  0   0
 * sin  0  cos 0
 *  0   0  0   1
 * @endcode
 */
brh_mat4 mat4_create_rotation_y(float theta);

/**
 * @brief Creates a rotation matrix around the z-axis.
 * 
 * A rotation matrix rotates the coordinates of a vector around the z-axis.
 * 
 * @param theta Angle of rotation in radians.
 * @return A 4x4 rotation matrix.
 * 
 * @code
 * cos -sin 0 0
 * sin cos 0 0
 *  0   0  1 0
 *  0   0  0 1
 * @endcode
 */
brh_mat4 mat4_create_rotation_z(float theta);

/**
 * @brief Creates a world matrix from translation, rotation, and scale vectors.
 * 
 * The world matrix is used to transform the coordinates of a vector from local space to world space.
 * 
 * @param translation Translation vector.
 * @param rotation Rotation vector (in radians).
 * @param scale Scaling vector.
 * @return A 4x4 world matrix.
 */
brh_mat4 mat4_create_world_matrix(brh_vector3 translation, brh_vector3 rotation, brh_vector3 scale);

/**
 * @brief Multiplies a 4x4 matrix by a 4D vector.
 * 
 * @param m Pointer to the 4x4 matrix.
 * @param v The 4D vector.
 * @return The resulting 4D vector.
 */
brh_vector4 mat4_mul_vec4(const brh_mat4* m, brh_vector4 v);

/**
 * @brief Multiplies a 4x4 matrix by a 4D vector and stores the result in the vector.
 * 
 * @param m Pointer to the 4x4 matrix.
 * @param v Pointer to the 4D vector.
 */
void mat4_mul_vec4_ref(const brh_mat4* m, brh_vector4* v);

/**
 * @brief Multiplies two 4x4 matrices and stores the result in a third matrix.
 * The order of multiplication is B * A. This is important because matrix multiplication is not commutative.
 * 
 * @param a Pointer to the first 4x4 matrix.
 * @param b Pointer to the second 4x4 matrix.
 * @param result Pointer to the resulting 4x4 matrix.
 */
void mat4_mul_mat4_ref(const brh_mat4* a, const brh_mat4* b, brh_mat4* result);