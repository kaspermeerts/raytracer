#ifndef CG_COLOUR_H
#define CG_COLOUR_H

#include <stdio.h>
#include <stdbool.h>

typedef struct Colour {
	float r, g, b, a;
} Colour;

Colour colour_add(Colour a, Colour b);
Colour colour_scale(float s, Colour a);
Colour colour_mul(Colour a, Colour b);

static const Colour BLACK = {0.0, 0.0, 0.0, 1.0};
static const Colour RED   = {1.0, 0.0, 0.0, 1.0};
static const Colour GREEN = {0.0, 1.0, 0.0, 1.0};
static const Colour BLUE  = {0.0, 0.0, 1.0, 1.0};

#endif
