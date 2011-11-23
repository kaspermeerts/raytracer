#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "colour.h"
#include "ray.h"
#include "ppm.h"

/* TODO: Put in config structure */
const int WIDTH =  512;
const int HEIGHT = 512;

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
