#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "colour.h"
#include "ray.h"
#include "shading.h"
#include "ppm.h"
#include "timer.h"

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
	fflush(stdout);
}

int main(int argc, char **argv)
{
	Timer *render_timer;
	Sdl *sdl;
	FILE *out;
	Colour *buffer;
	int width, height;

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	width = config->width;
	height = config->height;
	buffer = calloc(width*height, sizeof(Colour));

	srand(0x20071208);
	/* START */
	render_timer = timer_start("Rendering");

	for (int j = 0; j < height; j++)
	{
	for (int i = 0; i < width; i++) /* Indentation doesn't help readability */
	{
		Camera *cam = scene->camera;
		Colour c;
		Ray r;

		if (config->antialiasing)
		{
			c = BLACK;
			for (int k = 0; k < SQUARE(config->aa_samples); k++)
			{
				r = camera_ray_aa(cam, i, j, k, 1);

				c = colour_add(c, ray_colour(r, config->max_reflections));
			}
			c = colour_scale(1.0/SQUARE(config->aa_samples), c);
		} else
		{
			r = camera_ray(cam, i, j, 1);
			c = ray_colour(r, config->max_reflections);
		}

		buffer[width*j + i] = c;
	}
	print_progressbar(j, height - 1);
	}
	printf("\n");

	/* STOP */

	timer_stop(render_timer);
	timer_diff_print(render_timer);
	printf("%.2f kilopixels per second\n",
			width*height/1000./(timer_diff(render_timer)));
	out = fopen("ray.ppm", "w");
	ppm_write(buffer, width, height, out);
	free(buffer);
	fclose(out);

	return 0;
}
