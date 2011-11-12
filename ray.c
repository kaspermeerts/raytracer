#include <math.h>
#include "ray.h"

float ray_sphere_intersect(Ray r, Vec3 c, float radius)
{
	Vec3 v;
	float vd, v2, discriminant, t1, t2;

	v = vec3_sub(r.origin, c);
	v2 = vec3_dot(v, v);
	vd = vec3_dot(v, r.direction);

	discriminant = vd*vd - (v2 - radius*radius);
	if (discriminant < 0)
		return -HUGE_VAL;

	t1 = -vd + sqrt(discriminant);
	t2 = -vd - sqrt(discriminant);

	return MIN(t1, t2);
}

float ray_cylinder_intersect(Ray ray, float height, float radius)
{
	float a, b, c, disc, t1, t2, z;

	a = SQUARE(ray.direction.x) + SQUARE(ray.direction.y);
	b = 2*(ray.origin.x*ray.direction.x + ray.origin.y*ray.direction.y);
	c = SQUARE(ray.origin.x) + SQUARE(ray.origin.y) - SQUARE(radius);

	disc = b*b - 4*a*c;
	if (disc < 0)
		return -HUGE_VAL;

	t1 = (-b+sqrt(disc))/(2*a);
	t2 = (-b-sqrt(disc))/(2*a);
	/* t2 is the closest intersection? */

	z = ray.origin.z + t2*ray.direction.z;
	if (z < 0 || z > height)
		return -HUGE_VAL;

	printf("%g\n", z);
	return t2;
}
