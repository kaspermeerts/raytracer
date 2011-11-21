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
#include "ppm.h"

/* TODO: Put in config structure */
const int WIDTH =  512;
const int HEIGHT = 512;

static Colour diff_colour(Light *light, Material *mat,
		Vec3 cam_dir, Vec3 light_dir, Vec3 normal)
{
	Colour light_col;
	float ndotv;

	cam_dir = cam_dir;

	ndotv = MAX(0, vec3_dot(light_dir, normal));

	light_col = colour_scale(light->intensity, colour_scale(ndotv, light->colour));
	return colour_mul(mat->colour, light_col);
}

static Colour spec_colour(Light *light, Material *mat,
		Vec3 light_dir, Vec3 cam_dir, Vec3 normal)
{
	float hdotn;
	Vec3 half;
	Colour light_col;

	half = vec3_normalize(vec3_add(light_dir, cam_dir));
	hdotn = powf(MAX(0, vec3_dot(half, normal)), mat->shininess);

	light_col = colour_scale(light->intensity,
			colour_scale(hdotn, light->colour));
	return colour_mul(mat->colour, light_col);
}

static Colour light_mat_colour(Light *light, Material *mat,
		Vec3 cam_dir, Vec3 light_dir, Vec3 normal)
{
	Colour final, col1, col2;

	switch(mat->type)
	{
	case MATERIAL_DIFFUSE:
		return diff_colour(light, mat, cam_dir, light_dir, normal);
		break;
	case MATERIAL_PHONG:
		return spec_colour(light, mat, cam_dir, light_dir, normal);
		break;
	case MATERIAL_COMBINED:
		col1 = light_mat_colour(light, mat->mat1, cam_dir, light_dir, normal);
		col2 = light_mat_colour(light, mat->mat2, cam_dir, light_dir, normal);
		final = colour_add(
				colour_scale(mat->weight1, col1),
				colour_scale(mat->weight2, col2));
		break;
	default:
		assert("Unknown material!" == NULL);
	}

	return final;
}

static Colour hit_colour(Hit *hit, Vec3 cam_pos, int n, Light **lights,
		Material *mat)
{
	Colour total;

	total = BLACK;
	for (int i = 0; i < n; i++)
	{
		Light *light = lights[i];
		Vec3 cam_dir, light_dir;
		Colour new;

		cam_dir = vec3_normalize(vec3_sub(cam_pos, hit->position));
		light_dir = vec3_normalize(vec3_sub(light->position, hit->position));

		new = light_mat_colour(light, mat, cam_dir, light_dir, hit->normal);
		total = colour_add(total, new);
	}

	return total;
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

	return hit_colour(&hit, scene->camera->position, scene->num_lights,
			scene->light, mat);
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
	printf("%.2f kilopixels per second\n", 
			(WIDTH*HEIGHT/1000.0/(ms/1000.0)));

	out = fopen("ray.ppm", "w");
	ppm_write(buffer, WIDTH, HEIGHT, out);
	free(buffer);
	fclose(out);

	return 0;
}
