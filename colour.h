#ifndef CG_COLOUR_H
#define CG_COLOUR_H

#include <stdio.h>
#include <stdbool.h>

typedef struct Colour {
	float r, g, b, a;
} Colour;

bool write_image(Colour *buffer, int width, int height, FILE *fd);
#endif
