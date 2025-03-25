#pragma once

#include <stdint.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

uint32_t gcd(uint32_t a, uint32_t b);
