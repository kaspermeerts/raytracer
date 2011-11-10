#include <math.h>
#include "ray.h"

float ray_sphere_intersect(Ray r, Sphere s)
{
	Vec3 c = s.center;
	Vec3 v;
	float vd, v2, discriminant, t1, t2;

	v = vec3_sub(r.origin, c);
	v2 = vec3_dot(v, v);
	vd = vec3_dot(v, r.direction);

	discriminant = vd*vd - (v2 - s.radius*s.radius);
	if (discriminant < 0)
		return -HUGE_VAL;

	t1 = -vd + sqrt(discriminant);
	t2 = -vd - sqrt(discriminant);

	return MIN(t1, t2);
}
