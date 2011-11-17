#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cgmath.h"
#include "colour.h"
#include "scene.h"
#include "ray.h"

/* TODO: Put in config structure */
const int WIDTH = 256;
const int HEIGHT = 256;

static Colour diffuse_light(int n, Light **lights, Vec3 cam_pos,
		Vec3 hit_pos, Vec3 normal)
{
	Colour total;
	int i;

	total.r = total.g = total.b = total.a = 0.0;
	for (i = 0; i < n; i++)
	{
		float ndotl;
		Light *light = lights[i];
		Vec3 light_dir;

		light_dir = vec3_normalize(vec3_sub(light->position, hit_pos));
		ndotl = MAX(0, vec3_dot(normal, light_dir));

		total = colour_add(total, colour_scale(light->intensity,
				colour_scale(ndotl, light->colour)));
	}

	return total;
}

static Colour specular_light(int n, Light **lights, Vec3 cam_pos,
		Vec3 hit_pos, Vec3 normal, int shininess)
{
	Colour total;
	int i;

	total.r = total.g = total.b = total.a = 0.0;
	for (i = 0; i < n; i++)
	{
		Light *light = lights[i];
		Vec3 light_dir, cam_dir, half;
		float hdotn;

		light_dir = vec3_normalize(vec3_sub(light->position, hit_pos));
		cam_dir = vec3_normalize(vec3_sub(cam_pos, hit_pos));
		half = vec3_normalize(vec3_add(light_dir, cam_dir));
		hdotn = pow(MAX(0, vec3_dot(half, normal)), shininess);

		total = colour_add(total, colour_scale(light->intensity, colour_scale(hdotn, light->colour)));
	}

	return total;
}

static Colour material_colour(int n, Light **lights, Material *mat,
		Hit *hit, Camera *cam)
{
	Colour final, light_col, col1, col2;

	switch(mat->type)
	{
	case MATERIAL_DIFFUSE:
		light_col = diffuse_light(n, lights, cam->position,
				hit->position, hit->normal);
		final = colour_mul(mat->colour, light_col);
		break;
	case MATERIAL_PHONG:
		light_col = specular_light(n, lights, cam->position,
				hit->position, hit->normal,	mat->shininess);
		final = colour_mul(mat->colour, light_col);
		break;
	case MATERIAL_COMBINED:
			col1 = material_colour(n, lights, mat->mat1, hit, cam);
			col2 = material_colour(n, lights, mat->mat2, hit, cam);
			final = colour_add(
					colour_scale(mat->weight1, col1),
					colour_scale(mat->weight2, col2));
		break;
	default:
		printf("Huh?\n");
		assert(0);
	}

	return final;
}

static Colour ray_colour(Ray ray, Scene *scene, int ttl)
{
	Hit hit;
	bool ray_hit;
	Material *mat;

	/* Early exit */
	if (ttl <= 0)
		return scene->background;

	ray_hit = ray_intersect(ray, scene, &hit);

	if (!ray_hit)
		return scene->background;

	mat = hit.surface->material;

	return material_colour(scene->num_lights, scene->light, mat, 
			&hit, scene->camera);
}

int main(int argc, char **argv)
{
	Sdl *sdl;
	Scene *scene;
	FILE *out;
	Colour *buffer;
	clock_t start, stop;
	double ms;

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;
	scene = &sdl->scene;

	buffer = calloc(WIDTH*HEIGHT, sizeof(Colour));

	/* START */
	start = clock();

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

	/* STOP */
	stop = clock();

	ms = ((stop - start)*1000.0)/CLOCKS_PER_SEC;
	printf("Rendering complete in %d s %03d ms\n", (int) ms/1000, 
			(int) (ms - floor(ms/1000)*1000) );
	printf("%.2f megapixels per second\n", 
			(WIDTH*HEIGHT/1000000.0/(ms/1000.0)));

	out = fopen("test.ppm", "w");
	write_image(buffer, WIDTH, HEIGHT, out);
	free(buffer);
	fclose(out);

	return 0;
}
