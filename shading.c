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
		int p, q;
		float alpha, beta;
		p = j % n;
		q = j / n;
		alpha = p / (float) n + drand();
		beta =  q / (float) n + drand();
		if (light->type == LIGHT_AREA)
			light_pos = vec3_add(vec3_add(light->position,
					vec3_scale(alpha, light->plane.edge1)),
					vec3_scale(beta, light->plane.edge2));
		else
			light_pos = light->position;

		light_dir = vec3_normalize(vec3_sub(light_pos, hit->position));

		shadow_ray.direction = light_dir;
		shadow_ray.origin = vec3_add(hit->position,
				vec3_scale(1e-1, shadow_ray.direction));
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

Colour ray_colour(Ray ray, int ttl)
{
	Hit hit;
	Colour total;
	Vec3 cam_dir;

	if (ttl < 0)
		return BLACK;

	if (!ray_intersect(ray, &hit))
		return scene->background;


	cam_dir = vec3_normalize(vec3_scale(-1, ray.direction));


	total = BLACK;
	for (int i = 0; i < scene->num_lights; i++)
		total = colour_add(total, hit_light_colour(&hit, scene->light[i], cam_dir));

	/*
	if (hit.surface->material->reflect > 0.0)
	{
		Ray rray;
		Colour reflect_colour;

		rray.direction = vec3_reflect(ray.direction, hit.normal);
		rray.origin = vec3_add(hit.position, vec3_scale(1e-4, rray.direction));
		rray.near = 0;
		rray.far = HUGE_VAL;

		reflect_colour = ray_colour(rray, ttl - 1);
		total = colour_add(total, colour_scale(hit.surface->material->reflect, colour_mul(hit.surface->material->specular_colour, reflect_colour)));
	}
	*/

	return total;
}
