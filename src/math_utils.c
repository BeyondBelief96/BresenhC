#include "math_utils.h"


uint32_t gcd(uint32_t a, uint32_t b) {
	while (b != 0) {
		uint32_t temp = b;
		b = a % b;
		a = temp;
	}
	return a;
}

void swap_int(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

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
float interpolate_x_from_y(int x0, int y0, int x1, int y1, int y)
{
	return x0 + (x1 - x0) * (float)(y - y0) / (float)(y1 - y0);
}

float calculate_slope(int x0, int y0, int x1, int y1)
{
	return (float)(y1 - y0) / (float)(x1 - x0);
}

float calculate_inverse_slope(int x0, int y0, int x1, int y1)
{
	return (float)(x1 - x0) / (float)(y1 - y0);
}

float degrees_to_radians(float degrees)
{
	return degrees * ( M_PI / 180.0f);
}