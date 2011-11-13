#ifndef CG_RAY_H
#define CG_RAY_H

#include "cgmath.h"
#include "scene.h"

typedef struct Ray {
	Vec3 origin;
	Vec3 direction; /* Not necessarily normalized */
} Ray;

typedef struct Hit {
	Surface *surface;
	Vec3 normal;
	float t; /* Parameter of the ray equation: v = o + t*d */
} Hit;

Ray camera_ray(Camera *cam, int nx, int ny, int i, int j, double near);
bool ray_intersect(Ray ray, Scene *scene, Hit *hit);
#endif
