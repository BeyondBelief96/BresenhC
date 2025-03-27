#pragma once

#include <stdint.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)

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
