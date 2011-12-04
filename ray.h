#ifndef CG_RAY_H
#define CG_RAY_H

#include "scene.h"
#include "cgmath.h"

typedef struct Ray {
	Vec3 origin;
	Vec3 direction; /* Not necessarily normalized */
	float near;
	float far;
} Ray;

typedef struct Hit {
	Surface *surface;
	Vec3 position;
	Vec3 normal;
	float t; /* Parameter of the ray equation: v = o + t*d */
} Hit;


float drand(void); /* TODO FIXME XXX THIS DOESN'T BELONG HERE */
Ray camera_ray_aa(Camera *cam, int i, int j, int sample, double near);
Ray camera_ray(Camera *cam, int i, int j, double near);
bool ray_intersect(Ray ray, Hit *hit);
#endif
