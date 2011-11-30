#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "colour.h"
#include "ray.h"
#include "ppm.h"

static void print_progressbar(int progress, int total)
{
	int bars;
	const int tot_bars = 70;

	bars = (progress * tot_bars)/total;

	printf("\r");
	printf("[");
	for (int i = 0; i < bars; i++)
		printf("=");
	printf(">");
	for (int i = bars + 1; i <= tot_bars; i++)
		printf(" ");
	printf("]");
	printf("\r");

}

int main(int argc, char **argv)
{
	Sdl *sdl;
	FILE *out;
	Colour *buffer;
	clock_t start, stop;
	double ms;
	int width, height;

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	width = scene->camera->width;
	height = scene->camera->height;
	buffer = calloc(width*height, sizeof(Colour));

	/* START */
	start = clock();

	for (int j = 0; j < height; j++)
	{
	for (int i = 0; i < width; i++) /* Indentation doesn't help readability */
	{
		Camera *cam = scene->camera;
		Colour c;
		Ray r;

		/* The last parameter is the near plane, which is irrelevant for
		 * the moment. */
		r = camera_ray(cam, width, height, i, j, 1);

		c = ray_colour(r, 10);

		buffer[width*j + i] = c;
	}
	print_progressbar(j, height - 1);
	}
	printf("\n");

	/* STOP */
	stop = clock();

	ms = ((stop - start)*1000.0)/CLOCKS_PER_SEC;
	printf("Rendering complete in %d s %03d ms\n", (int) ms/1000, 
			(int) (ms - floor(ms/1000)*1000) );
	printf("%.2f kilopixels per second\n", 
			(width*height/1000.0/(ms/1000.0)));

	out = fopen("ray.ppm", "w");
	ppm_write(buffer, width, height, out);
	free(buffer);
	fclose(out);

	return 0;
}
