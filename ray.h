#ifndef CG_RAY_H
#define CG_RAY_H

#include "cgmath.h"
#include "scene.h"

typedef struct Ray {
	Vec3 origin;
	Vec3 direction; /* Make sure this is normalized! */
} Ray;

float ray_sphere_intersect(Ray r, Vec3 c, float radius);
float ray_cylinder_intersect(Ray r, float height, float radius);
#endif
