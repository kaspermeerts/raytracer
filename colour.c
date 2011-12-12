#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "colour.h"

Colour colour_add(Colour a, Colour b)
{
	Colour c;
	c.r = a.r + b.r;
	c.g = a.g + b.g;
	c.b = a.b + b.b;
	c.a = a.a + b.a;

	return c;
}

Colour colour_mul(Colour a, Colour b)
{
	Colour c;
	c.r = a.r * b.r;
	c.g = a.g * b.g;
	c.b = a.b * b.b;
	c.a = a.a * b.a;

	return c;
}

Colour colour_scale(float s, Colour a)
{
	Colour b;
	b.r = s * a.r;
	b.g = s * a.g;
	b.b = s * a.b;
	b.a = s * a.a;

	return b;
}

Colour *colour_buffer_from_rgb(unsigned char *src,
		int width, int height)
{
	Colour *dst = calloc(width * height, sizeof(Colour));

	for (int j = 0; j < height; j++)
	for (int i = 0; i < width; i++)
	{
		int jj = height - 1 - j;
		dst[width*jj + i].r = src[3*(j*width + i) + 0] / 255.;
		dst[width*jj + i].g = src[3*(j*width + i) + 1] / 255.;
		dst[width*jj + i].b = src[3*(j*width + i) + 2] / 255.;
		dst[width*jj + i].a = 1;
	}

	return dst;
}
Colour *colour_buffer_from_rgba(unsigned char *src,
		int width, int height)
{
	Colour *dst = calloc(width * height, sizeof(Colour));

	for (int j = 0; j < height; j++)
	for (int i = 0; i < width; i++)
	{
		int jj = height - 1 - j;
		dst[width*jj + i].r = src[4*(j*width + i) + 0] / 255.;
		dst[width*jj + i].g = src[4*(j*width + i) + 1] / 255.;
		dst[width*jj + i].b = src[4*(j*width + i) + 2] / 255.;
		dst[width*jj + i].a = src[4*(j*width + i) + 3] / 255.;
	}

	return dst;
}
