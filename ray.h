#ifndef CG_RAY_H
#define CG_RAY_H

#include "cgmath.h"
#include "scene.h"

typedef struct Ray {
	Vec3 origin;
	Vec3 direction; /* Make sure this is normalized! */
} Ray;

Ray camera_ray(Camera *cam, int nx, int ny, int i, int j, double near);
float ray_sphere_intersect(Ray r, Vec3 c, float radius);
float ray_cylinder_intersect(Ray r, float height, float radius);
bool ray_intersect(Ray ray, Scene *scene, Surface **surface);
#endif
