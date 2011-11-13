#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cgmath.h"
#include "colour.h"
#include "scene.h"
#include "ray.h"

/* TODO: Put in config structure */
const int WIDTH = 500;
const int HEIGHT = 500;

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
		Colour *c = &buffer[WIDTH*j + i];
		Surface *surf;
		Ray r;
		bool hit;

		/* Step 1: Ray generating */
		r = camera_ray(cam, WIDTH, HEIGHT, i, j, 1);

		/* Step 2: Ray tracing */
		hit = ray_intersect(r, &sdl->scene, &surf);

		/* Step 3: Shading */
		if (hit)
			*c = surf->material->colour;
		else
			c->r = c->g = c->b = 0;
	}

	out = fopen("test.ppm", "w");
	write_image(buffer, WIDTH, HEIGHT, out);
	free(buffer);
	fclose(out);

	return 0;
}
