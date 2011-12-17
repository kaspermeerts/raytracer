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
#include "timer.h"

SDL_Surface *display_surface;
SDL_Surface *blit_surface;

typedef struct Pixel {
	int x;
	int y;
} Pixel;

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
	display_surface = SDL_SetVideoMode(config->width, config->height, 32, flags);
	if (!display_surface)
	{
		printf("Couldn't set videomode: %s\n", SDL_GetError());
		return false;
	}

	rmask = 0x000000FF;
	gmask = 0x0000FF00;
	bmask = 0x00FF0000;
	/* amask = 0xFF000000; */
	blit_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, config->width,
			config->height, 32, rmask, gmask, bmask, 0);

	return true;
}

static void put_pixel(SDL_Surface *surface, int x, int y, Colour c)
{
	int bpp = surface->format->BytesPerPixel;
	uint8_t *p, r, g, b;

	y = config->height - 1 - y;

	p = (uint8_t *)surface->pixels + y * surface->pitch +x * bpp;
	r = CLAMP(floorf(c.r * 256), 0, 255);
	g = CLAMP(floorf(c.g * 256), 0, 255);
	b = CLAMP(floorf(c.b * 256), 0, 255);

	*(uint32_t *)p = SDL_MapRGB(surface->format, r, g, b);
}

static void shuffle_pixels(Pixel *pixels, int w, int h)
{
	const int n = w*h;

	for (int i = n - 1; i >= 1; i--)
	{
		int j = rand() % (i + 1);
		Pixel dummy;
		dummy = pixels[i];
		pixels[i] = pixels[j];
		pixels[j] = dummy;
	}
}

int main(int argc, char **argv)
{
	Timer *render_timer;
	Sdl *sdl;
	FILE *out;
	Colour *buffer;
	Pixel *pixels;
	int num_pixels;

	SDL_Event event = {0};

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	if (!init_SDL())
		return 1;

	num_pixels = config->width * config->height;
	buffer = calloc(num_pixels, sizeof(Colour));
	pixels = calloc(num_pixels, sizeof(Pixel));
	for (int j = 0; j < config->height; j++)
	for (int i = 0; i < config->width; i++)
	{
		pixels[j*config->width + i].x = i;
		pixels[j*config->width + i].y = j;
	}
	srand(time(NULL));
	shuffle_pixels(pixels, config->width, config->height);
	srand(0x20071208);

	/* START */
	render_timer = timer_start("Rendering");

	for (int i = 0; i < num_pixels; i++)
	{
		Camera *cam = scene->camera;
		Colour c;
		Ray r;
		int x = pixels[i].x, y = pixels[i].y;

		/* The last parameter is the near plane, which is irrelevant for
		 * the moment. */
		r = camera_ray(cam, x, y, 1);

		c = ray_colour(r, 10);

		buffer[i] = c;
		put_pixel(display_surface, x, y, c);
		if (i % config->width == 0)
		{
			SDL_Flip(display_surface);
			while (SDL_PollEvent(&event))
				if (event.type == SDL_QUIT)
					return 0;
		}
	}

	/* STOP */
	timer_stop(render_timer);
	timer_diff_print(render_timer);
	printf("%.2f kilopixels per second\n",
			num_pixels/1000./(timer_diff(render_timer)));

	out = fopen("ray.ppm", "w");
	ppm_write(buffer, config->width, config->height, out);
	free(buffer);
	fclose(out);

	SDL_Flip(display_surface);
	while(1)
	{
		while (SDL_WaitEvent(&event))
			if (event.type == SDL_QUIT)
				return 0;
			else if (event.type == SDL_VIDEOEXPOSE)
				SDL_Flip(display_surface);
	}

	return 0;
}
