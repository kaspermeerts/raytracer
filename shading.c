#include <math.h>

#include "shading.h"
#include "ray.h"
#include "colour.h"

static Colour hit_light_colour(Hit *hit, Light *light, Vec3 cam_dir)
{
	Material *mat = hit->surface->material;
	Vec3 normal = hit->normal;
	Vec3 light_dir;
	Vec3 light_pos;
	Colour light_total;
	int n;

	light_total = BLACK;
	/* A point light is an area light with only one samples */
	n = light->type == LIGHT_AREA ? config->shadow_samples : 1;
	for (int j = 0; j < SQUARE(n); j++)
	{
		Ray shadow_ray;
		Hit dummy;
		Colour diff_col, spec_col;

		if (light->type == LIGHT_AREA)
		{
			int p, q;
			float alpha, beta;
			p = j % n;
			q = j / n;
			alpha = p / (float) n + drand();
			beta =  q / (float) n + drand();

			light_pos = vec3_add(vec3_add(light->position,
					vec3_scale(alpha, light->plane.edge1)),
					vec3_scale(beta, light->plane.edge2));
		}
		else
			light_pos = light->position;

		light_dir = vec3_normalize(vec3_sub(light_pos, hit->position));

		shadow_ray.direction = light_dir;
		shadow_ray.origin = vec3_add(hit->position,
				vec3_scale(1e-2, shadow_ray.direction));
		shadow_ray.near = 0;
		shadow_ray.far = vec3_length(vec3_sub(light_pos, hit->position));
		if (ray_intersect(shadow_ray, &dummy))
			continue;

		diff_col = diff_colour(light, mat, cam_dir, light_dir, normal);
		spec_col = spec_colour(light, mat, cam_dir, light_dir, normal);
		light_total = colour_add(light_total, colour_add(diff_col, spec_col));
	}
	light_total = colour_scale(1.0/SQUARE(n), light_total);

	return light_total;
}

static Vec3 vec3_orthogonal_vec3(Vec3 v)
{
	const Vec3 n1 = (Vec3) {1, 0, 0}, n2 = (Vec3) {0, 1, 0};

	if (fabs(vec3_dot(v, n1)) < vec3_length(v)/M_SQRT2)
		return vec3_cross(v, n1);
	else
		return vec3_cross(v, n2);
}

static Colour hit_reflection_colour(Hit *hit, Ray ray, int depth)
{
	Colour total;
	Material *mat = hit->surface->material;
	Ray rray;

	/* Non-reflecting material */
	if (mat->reflect <= 0.0)
		return BLACK;

	/* First, create the unperturbed reflection ray */
	rray.direction = vec3_reflect(ray.direction, hit->normal);
	rray.origin = vec3_add(hit->position, vec3_scale(1e-2, rray.direction));
	rray.near = 0;
	rray.far = HUGE_VAL;

	/* Only gloss primary and the first reflected rays.
	 * This is a crude form of importance sampling */
	if (mat->glossiness <= 0.0 || depth > 1)
		total = ray_colour(rray, depth + 1);
	else
	{
		total = BLACK;
		for (int i = 0; i < config->reflection_samples; i++)
		{
			Ray pray = rray; /* Perturbed ray */
			Vec3 a, b;

			/* The ray direction needs to be normalized for this to work */
			pray.direction = vec3_normalize(pray.direction);
			/* Create an orthonormal basis for the tangent vector space */
			a = vec3_normalize(vec3_orthogonal_vec3(pray.direction));
			b = vec3_normalize(vec3_cross(pray.direction, a));

			a = vec3_scale(mat->glossiness * (2*drand() - 1), a);
			b = vec3_scale(mat->glossiness * (2*drand() - 1), b);
			pray.direction = vec3_add(pray.direction, vec3_add(a, b));
			total = colour_add(total, ray_colour(pray, depth + 1));
		}
		total = colour_scale(1./config->reflection_samples, total);
	}
	return colour_mul(mat->specular_colour, colour_scale(mat->reflect, total));
}

Colour ray_colour(Ray ray, int depth)
{
	Hit hit;
	Colour total;
	Vec3 cam_dir = vec3_normalize(vec3_scale(-1, ray.direction));

	if (depth > config->max_reflections)
		return BLACK;

	if (!ray_intersect(ray, &hit))
	{
		if (scene->environment_map)
			return cubemap_colour(scene->environment_map, ray.direction);
		else
			return scene->background;
	}

	total = BLACK;
	/* Direct contributions from light */
	for (int i = 0; i < scene->num_lights; i++)
		total = colour_add(total,
				hit_light_colour(&hit, scene->light[i], cam_dir));

	/* Indirect contributions from reflections */
	total = colour_add(total, hit_reflection_colour(&hit, ray, depth));

	return total;
}
