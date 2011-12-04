#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>

#include "cgmath.h"
#include "colour.h"
#include "ray.h"
#include "shading.h"
#include "ppm.h"

/* TODO: Put in config structure */
const int WIDTH =  600;
const int HEIGHT = 600;

SDL_Surface *display_surface;
SDL_Surface *blit_surface;

static bool init_SDL(void)
{
	int flags;
	const SDL_VideoInfo *vidinfo;
	uint32_t rmask, gmask, bmask; /*, amask; */

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL failed: %s\n", SDL_GetError());
		return false;
	}
	atexit(SDL_Quit);

	vidinfo = SDL_GetVideoInfo();
	if (!vidinfo)
	{
		printf("Video info failed: %s\n", SDL_GetError());
		return false;
	}

	flags = 0;
	flags |= SDL_SWSURFACE;
	display_surface = SDL_SetVideoMode(WIDTH, HEIGHT, 32, flags);
	if (!display_surface)
	{
		printf("Couldn't set videomode: %s\n", SDL_GetError());
		return false;
	}

	rmask = 0x000000FF;
	gmask = 0x0000FF00;
	bmask = 0x00FF0000;
	/* amask = 0xFF000000; */
	blit_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT, 32, rmask, gmask, bmask, 0);

	return true;
}

static void put_pixel(SDL_Surface *surface, int x, int y, Colour c)
{
	int bpp = surface->format->BytesPerPixel;
	uint8_t *p, r, g, b;

	y = HEIGHT - 1 - y;

	p = (uint8_t *)surface->pixels + y * surface->pitch +x * bpp;
	r = CLAMP(floorf(c.r * 256), 0, 255);
	g = CLAMP(floorf(c.g * 256), 0, 255);
	b = CLAMP(floorf(c.b * 256), 0, 255);

	*(uint32_t *)p = SDL_MapRGB(surface->format, r, g, b);
}

int main(int argc, char **argv)
{
	Sdl *sdl;
	FILE *out;
	Colour *buffer;
	clock_t start, stop;
	double ms;

	SDL_Event event = {0};

	if (argc < 2)
		return 1;

	if (!init_SDL())
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	buffer = calloc(WIDTH*HEIGHT, sizeof(Colour));

	/* START */
	start = clock();

	for (int j = 0; j < HEIGHT; j++)
	{
	for (int i = 0; i < WIDTH; i++) /* Indentation doesn't help readability */
	{
		Camera *cam = scene->camera;
		Colour c;
		Ray r;

		/* The last parameter is the near plane, which is irrelevant for
		 * the moment. */
		r = camera_ray(cam, i, j, 1);

		c = ray_colour(r, 10);

		buffer[WIDTH*j + i] = c;
		put_pixel(display_surface, i, j, c);
	}
		SDL_Flip(display_surface);
		while (SDL_PollEvent(&event))
			if (event.type == SDL_QUIT)
				return 0;
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
