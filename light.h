#ifndef CG_LIGHT
#define CG_LIGHT

#include "cgmath.h"

typedef struct Light {
	enum { DIRECTIONAL, POINT, SPOT } type;
	Vec3 position;
	Vec3 direction; /* Keep this normalized */
	float angle;
	Colour colour;
	char *name;
} Light;

#endif
