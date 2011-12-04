#include <math.h>

#include "shading.h"
#include "ray.h"
#include "colour.h"

Colour ray_colour(Ray ray, int ttl)
{
	Hit hit;
	Colour total;
	Material *mat;
	Vec3 cam_pos, cam_dir, normal;

	if (ttl <= 0)
	{
		printf("TTL ran out\n");
		return scene->background;
	}

	if (!ray_intersect(ray, &hit))
		return scene->background;


	cam_pos = scene->camera->position;
	cam_dir = vec3_normalize(vec3_sub(cam_pos, hit.position));
	normal = hit.normal;
	mat = hit.surface->material;

	total = BLACK;
	for (int i = 0; i < scene->num_lights; i++)
	{
		Light *light = scene->light[i];
		Vec3 light_dir;
		Vec3 light_pos;
		Colour diff_col, spec_col, light_total;
		Ray shadow_ray;
		Hit dummy;
		int n;

		light_total = BLACK;
		/* A point light is an area light with only one samples */
		n = light->type == LIGHT_AREA ? config->shadow_samples : 1;
		for (int j = 0; j < SQUARE(n); j++)
		{
			int p, q;
			float alpha, beta;
			p = j % n;
			q = j / n;
			alpha = p / (float) n + drand();
			beta =  q / (float) n + drand();
			if (light->type == LIGHT_AREA)
				light_pos = vec3_add(vec3_add(light->position, vec3_scale(alpha, light->plane.edge1)), vec3_scale(beta, light->plane.edge2));
			else
				light_pos = light->position;

			light_dir = vec3_normalize(vec3_sub(light_pos, hit.position));

			shadow_ray.direction = light_dir;
			shadow_ray.origin = vec3_add(hit.position,
					vec3_scale(1e-3, shadow_ray.direction));
			shadow_ray.near = 0;
			shadow_ray.far = vec3_length(vec3_sub(light_pos, hit.position));
			if (ray_intersect(shadow_ray, &dummy))
				continue;

			diff_col = diff_colour(light, mat, cam_dir, light_dir, normal);
			spec_col = spec_colour(light, mat, cam_dir, light_dir, normal);
			light_total = colour_add(light_total, colour_add(diff_col, spec_col));
		}
		light_total = colour_scale(1.0/SQUARE(n), light_total);
		total = colour_add(total, light_total);
	}

	return total;
}
