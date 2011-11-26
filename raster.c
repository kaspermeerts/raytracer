#include <math.h>
#include <stdlib.h>

#include "colour.h"
#include "raster.h"

Raster *raster_new(int width, int height)
{
	Raster *raster;

	raster = malloc(sizeof(Raster));
	raster->width = width;
	raster->height = height;
	raster->buffer = calloc(width*height, sizeof(Colour));
	raster->zbuffer = calloc(width*height, sizeof(raster->zbuffer[0]));

	return raster;
}

void raster_destroy(Raster *raster)
{
	free(raster->zbuffer);
	free(raster->buffer);
	free(raster);
}

bool raster_pixel(Raster *raster, int x, int y, Colour c)
{
	if (x < 0 || x >= raster->width || y < 0 || y >= raster->width)
		return false;

	raster->buffer[raster->width*y + x] = c;

	return true;
}

/* Bresenham's line algorithm. */
void raster_line(Raster *raster, int x0, int y0, int x1, int y1)
{
	int x, y;
	int deltax;
	int deltay;
	int error;
	bool steep;

#define SWAP(a, b) do {int dummy; dummy = (a); (a) = (b); (b) = dummy;} while(0)
	/* We only consider lines with a slope smaller than one. For steep lines,
	 * we switch the x and y coordinates and switch them again when plotting. */
	if (abs(y1 - y0) > abs(x1 - x0))
	{
		steep = true;
		SWAP(x0, y0);
		SWAP(x1, y1);
	} else
		steep = false;
	/* We only consider lines that go from left to right */
	if (x0 > x1)
	{
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
#undef SWAP

	deltax = x1 - x0;
	deltay = abs(y1 - y0);

	error = -deltax / 2;
	y = y0;
	for (x = x0; x <= x1; x++)
	{
		if (steep)
			raster_pixel(raster, y, x, RED);
		else
			raster_pixel(raster, x, y, RED);
		error += deltay;
		if (error >= 0)
		{
			if (y0 < y1)
				y++;
			else
				y--;
			error -= deltax;
		}
	}
}
