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
