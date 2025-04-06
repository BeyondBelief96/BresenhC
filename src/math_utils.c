#include "math_utils.h"

const float EPSILON = 1e-6f;

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

void swap_float(float* a, float* b)
{
	float temp = *a;
	*a = *b;
	*b = temp;
}

float interpolate_x_from_y(int x0, int y0, int x1, int y1, int y)
{
	return x0 + (x1 - x0) * (float)(y - y0) / (float)(y1 - y0);
}

float interpolate_float(float a, float b, float factor) {
	if (factor < 0.0f) factor = 0.0f;
	if (factor > 1.0f) factor = 1.0f;
	return a + (b - a) * factor;
}


float calculate_slope(int x0, int y0, int x1, int y1)
{
	if (x1 - x0 == 0)
	{
		return 0.0f; 
	}

	return (float)(y1 - y0) / (float)(x1 - x0);
}

float calculate_inverse_slope(int x0, int y0, int x1, int y1)
{
	if (y1 - y0 != 0)
	{
		return (float)(x1 - x0) / (float)(y1 - y0);
	}
	else
	{
		return 0.0f;
	}
}

float degrees_to_radians(float degrees)
{
	return degrees * ( (float)M_PI / 180.0f);
}