#pragma once

#include <stdint.h>
#include <math.h>

#define M_PI acos(-1.0)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern const float EPSILON;

uint32_t gcd(uint32_t a, uint32_t b);

/*
* @brief Swaps the values of two integers.
* 
* This function swaps the values of two integers using pointers.
* 
* @param a Pointer to the first integer.
* @param b Pointer to the second integer.
* 
* @return void
*/
void swap_int(int* a, int* b);

/*
* @brief Swaps the values of two floats.
* 
* This function swaps the values of two floats using pointers.
* 
* @param a Pointer to the first float.
* @param b Pointer to the second float.
* 
* @return void
*/
void swap_float(float* a, float* b);

/**
* @brief Interpolates between two float values.
* 
* This function performs linear interpolation between two float values based on a given factor.
* 
* @param a The first float value.
* @param b The second float value.
* @param factor The interpolation factor (0.0 to 1.0).
* 
* @return The interpolated float value.
*/
float interpolate_float(float a, float b, float factor);

/*
* @brief Interpolates the x value from a given y value.
*
* This function calculates the x value of a point given a y value using linear interpolation.
*
* @param x0 The x value of the first point.
* @param y0 The y value of the first point.
* @param x1 The x value of the second point.
* @param y1 The y value of the second point.
* @param y The y value of the point to interpolate.
*
* @return The x value of the interpolated point.
*/
float interpolate_x_from_y(int x0, int y0, int x1, int y1, int y);

/*
* @brief Calculates the slope of a line.
* 
* This function calculates the slope of a line given two points.
* 
* @param x0 The x value of the first point.
* @param y0 The y value of the first point.
* @param x1 The x value of the second point.
* @param y1 The y value of the second point.
* 
* @return The slope of the line.
*/
float calculate_slope(int x0, int y0, int x1, int y1);

/*
* @brief Calculates the inverse slope of a line.
*
* This function calculates the slope of a line given two points.
*
* @param x0 The x value of the first point.
* @param y0 The y value of the first point.
* @param x1 The x value of the second point.
* @param y1 The y value of the second point.
*
* @return The slope of the line.
*/
float calculate_inverse_slope(int x0, int y0, int x1, int y1);

/*
* @brief Converts degrees to radians.
* 
* This function converts an angle from degrees to radians.
* 
* @param degrees The angle in degrees.
* 
* @return The angle in radians.
*/
float degrees_to_radians(float degrees);
