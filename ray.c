#include <assert.h>
#include <math.h>
#include "ray.h"

Ray camera_ray(Camera *cam, int nx, int ny, int i, int j, double near)
{
	float d, u, v;
	Ray r;
	double bottom, left, width, height;

	width = near*tan(cam->fov*M_TWO_PI/360.);
	left = -width/2;
	height = width * ny/(double) nx;
	bottom = -height/2;

	d = near;
	u = left + width*(i + 0.5)/nx;
	v = bottom + height*(j + 0.5)/ny;
	r.origin = cam->position;
	r.direction = vec3_normalize(vec3_add(
			vec3_scale(-d, cam->w), vec3_add(
			vec3_scale( u, cam->u),
			vec3_scale( v, cam->v))));

	return r;
}

static bool ray_sphere_intersect(Ray r, Sphere sph, float *t)
{
	Vec3 v;
	float dd, vd, vv, discriminant, t1, t2;
	float radius = sph.radius;

	v = r.origin;
	vv = vec3_dot(v, v);
	vd = vec3_dot(v, r.direction);
	dd = vec3_dot(r.direction, r.direction);

	discriminant = vd*vd - dd*(vv - radius*radius);
	if (discriminant < 0)
		return false;

	t1 = (-vd + sqrt(discriminant))/dd;
	t2 = (-vd - sqrt(discriminant))/dd;

	*t = t2;
	return true;
}

static float ray_cylinder_intersect(Ray ray, Cylinder cyl)
{
	float height = cyl.height, radius = cyl.radius;
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

	return t2;
}

bool ray_intersect(Ray ray, Scene *scene, Hit *hit)
{
	float t;
	Node *hit_node = &scene->graph;
	Surface *surf = &hit_node->u.surface; /* Necessarily a surface node */

	assert(hit_node->type == NODE_SURFACE);

	hit->surface = surf;
	switch(surf->shape->type)
	{
	case SHAPE_CYLINDER:
		t = ray_cylinder_intersect(ray, surf->shape->u.cylinder);
		if (t > 0)
		{
			hit->t = t;
			return true;
		}
		break;
	case SHAPE_SPHERE:
		if (ray_sphere_intersect(ray, surf->shape->u.sphere, &hit->t))
		{
			hit->position = vec3_add(ray.origin, vec3_scale(hit->t, ray.direction));
			hit->normal = vec3_normalize(hit->position);
			return true;
		}
		break;
	default:
		printf("What?\n");
		break;
	}

	return false;
}
