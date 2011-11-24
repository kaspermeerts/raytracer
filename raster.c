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

void raster_line(Raster *raster, int x0, int y0, int x1, int y1)
{
	int x, y;

	y = y0;
	for (x = x0; x <= x1; x++)
	{
		RASTER_PIXEL(*raster, x, y).r = 255;
	}
}
