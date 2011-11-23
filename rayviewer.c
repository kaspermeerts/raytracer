#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>

#include "cgmath.h"
#include "colour.h"
#include "ray.h"
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
	uint32_t rmask, gmask, bmask, amask;

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
	flags |= SDL_HWSURFACE;
	flags |= SDL_DOUBLEBUF;
	display_surface = SDL_SetVideoMode(WIDTH, HEIGHT, 24, flags);
	if (!display_surface)
	{
		printf("Couldn't set videomode: %s\n", SDL_GetError());
		return false;
	}

	rmask = 0x000000FF;
	gmask = 0x0000FF00;
	bmask = 0x00FF0000;
	amask = 0xFF000000;
	blit_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT, 24, rmask, gmask, bmask, 0);

	return true;
}

static void put_pixel(SDL_Surface *surface, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	int bpp = surface->format->BytesPerPixel;
	uint8_t *p;

	y = HEIGHT - 1 - y;

	p = (uint8_t *)surface->pixels + y * surface->pitch +x * bpp;

	if (bpp == 3)
	{
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		{
			printf("Fuck\n");
			exit(1);
		} else
		{
			p[0] = b;
			p[1] = g;
			p[2] = r;
		}
	} else if (bpp == 4)
	{
		p[0] = 0xFF;
		p[1] = b;
		p[2] = g;
		p[3] = r;
	} else
	{
		printf("Unsupported colour depth %d\n", bpp);
		exit(1);
	}
}

static void blit_buffer(Colour *buffer)
{
	int i, j;


	for (j = 0; j < HEIGHT; j++)
	for (i = 0; i < WIDTH;  i++)
	{
		Colour c = buffer[j*WIDTH + i];
		uint8_t r, g, b, a;

		r = CLAMP(floorf(c.r * 256), 0, 255);
		g = CLAMP(floorf(c.g * 256), 0, 255);
		b = CLAMP(floorf(c.b * 256), 0, 255);
		a = 255;

		put_pixel(blit_surface, i, j, r, g, b);
	}

	SDL_BlitSurface(blit_surface, NULL, display_surface, NULL);

	return;
}

int main(int argc, char **argv)
{
	Sdl *sdl;
	Scene *scene;
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
	scene = &sdl->scene;

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
		r = camera_ray(cam, WIDTH, HEIGHT, i, j, 1);

		c = ray_colour(r, &sdl->scene, 10);

		buffer[WIDTH*j + i] = c;
	}
		while (SDL_PollEvent(&event))
			if (event.type == SDL_QUIT)
				return 0;
		if (j % 20 == 0)
		{
			blit_buffer(buffer);
			SDL_Flip(display_surface);
		}
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
