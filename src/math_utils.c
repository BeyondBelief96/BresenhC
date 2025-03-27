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