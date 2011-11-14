#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cgmath.h"
#include "colour.h"
#include "scene.h"
#include "ray.h"

/* TODO: Put in config structure */
const int WIDTH = 512;
const int HEIGHT = 512;

static Colour diffuse_light(int n, Light **lights, Vec3 position, Vec3 normal)
{
	Colour total;
	int i;

	total.r = total.g = total.b = total.a = 0.0;
	for (i = 0; i < n; i++)
	{
		float ndotl;
		Vec3 light_dir;
		Light *light = lights[i];

		light_dir = vec3_normalize(vec3_sub(light->position, position));
		ndotl = MAX(0, vec3_dot(normal, light_dir));

		total = colour_add(total, colour_scale(light->intensity,
				colour_scale(ndotl, light->colour)));
	}

	return total;
}

static Colour ray_colour(Ray ray, Scene *scene, int ttl)
{
	Hit hit;
	bool ray_hit;
	Material *mat;
	Colour c, light_col;

	/* Early exit */
	if (ttl <= 0)
		return scene->background;

	ray_hit = ray_intersect(ray, scene, &hit);

	if (!ray_hit)
		return scene->background;

	mat = hit.surface->material;
	switch(mat->type)
	{
	case MATERIAL_DIFFUSE:
		light_col = diffuse_light(scene->num_lights, scene->light,
				hit.position, hit.normal);
		c = colour_mul(mat->colour, light_col);
		break;
	case MATERIAL_PHONG:
		break;
	default:
		break;
	}

	return c;
}

int main(int argc, char **argv)
{
	Sdl *sdl;
	Scene *scene;
	FILE *out;
	Colour *buffer;

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;
	scene = &sdl->scene;

	buffer = calloc(WIDTH*HEIGHT, sizeof(Colour));

	for (int j = 0; j < HEIGHT; j++)
	for (int i = 0; i < WIDTH; i++) /* Indentation doesn't help readability */
	{
		Camera *cam = scene->camera;
		Colour c;
		Ray r;

		/* The last parameter is the near plane, which is irrelevant for
		 * the moment. */
		r = camera_ray(cam, WIDTH, HEIGHT, i, j, 1);

		c = ray_colour(r, &sdl->scene, 10);

		buffer[WIDTH*j + i] = c;
	}

	out = fopen("test.ppm", "w");
	write_image(buffer, WIDTH, HEIGHT, out);
	free(buffer);
	fclose(out);

	return 0;
}
