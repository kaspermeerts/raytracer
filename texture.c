#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "pnglite/pnglite.h"
#include "colour.h"
#include "texture.h"

static Colour *buffer_from_png(const char *filename, int *width, int *height)
{
	unsigned char *data;
	Colour *buffer;
	png_t png;

	png_init(NULL, NULL);

	if (png_open_file_read(&png, filename) != PNG_NO_ERROR)
	{
		printf("Couldn't open file %s\n", filename);
		return NULL;
	}
	*width = png.width;
	*height = png.height;
	assert(png.bpp == 4 || png.bpp == 3);

	data = calloc(png.width * png.height * png.bpp, 1);
	int ret = png_get_data(&png, data);
	if (ret != PNG_NO_ERROR)
	{
		printf("%d\n", ret);
		free(data);
		printf("Error getting data from png %s\n", filename);
		return NULL;
	}

	if (png.bpp == 4)
		buffer = colour_buffer_from_rgba(data, png.width, png.height);
	else
		buffer = colour_buffer_from_rgb(data, png.width, png.height);

	free(data);

	return buffer;
}

CubeMap *cubemap_load(const char *prefix)
{
	Colour *dummy;
	CubeMap *map;
	char filename[1024];
	int w, h;


	sprintf(filename, "%s_%s.png", prefix, cube_direction[0]);
	if((dummy = buffer_from_png(filename, &w, &h)) == NULL)
		return NULL;

	map = malloc(sizeof(*map));
	map->width = w;
	map->height = h;

	for (int i = 0; i < 6; i++)
	{
		sprintf(filename, "%s_%s.png", prefix, cube_direction[i]);
		map->buffer[i] = buffer_from_png(filename, &w, &h);
		if (map->buffer[i] == NULL)
		{
			free(map);
			return NULL;
		}
		if (w != map->width || h != map->height)
		{
			printf("File %s has an inconsistent width or height: %d %d\n",
					filename, w, h);
			free(map);
			return NULL;
		}
	}
	return map;
}
