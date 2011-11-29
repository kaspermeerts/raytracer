#include <math.h>
#include <stdbool.h>
#include <stdio.h>

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
