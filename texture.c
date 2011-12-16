#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "pnglite/pnglite.h"
#include "cgmath.h"
#include "colour.h"
#include "texture.h"

const char *cube_direction_str[] = {
		"negative_x", "positive_x", "negative_y", "positive_y",
		"negative_z", "positive_z" };

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

Texture *texture_load_png(const char *filename)
{
	int w, h;
	Colour *buf;
	Texture *tex;

	if ((buf = buffer_from_png(filename, &w, &h)) == NULL)
	{
		printf("Couldn't load texture %s\n", filename);
		return NULL;
	}
	tex = malloc(sizeof(Texture));
	tex->buffer = buf;
	tex->width = w;
	tex->height = h;

	return tex;
}

Colour texture_texel(Texture *texture, double u, double v)
{
	int x0, y0;
	int x1, y1;
	double ualpha, ubeta, valpha, vbeta;
	Colour caa, cab, cba, cbb;

	if (u == 1.0)
		u -= 1e-6;
	if (v == 1.0)
		v -= 1e-6;
	u = u - floor(u);
	v = v - floor(v);
	u *= texture->width;
	v *= texture->height;
	x0 = floor(u);
	y0 = floor(v);

	ualpha = u - x0;
	valpha = v - y0;
	ubeta = 1 - ualpha;
	vbeta = 1 - valpha;

	if (x0 == texture->width - 1)
		x1 = x0;
	else
		x1 = x0 + 1;

	if (y0 == texture->height - 1)
		y1 = y0;
	else
		y1 = y0 + 1;

	caa = texture->buffer[texture->width*y0 + x0];
	cab = texture->buffer[texture->width*y0 + x1];
	cba = texture->buffer[texture->width*y1 + x0];
	cbb = texture->buffer[texture->width*y1 + x1];

	return colour_add(
			colour_scale(vbeta,
				colour_add(colour_scale(ubeta, caa), colour_scale(ualpha, cab))),
			colour_scale(valpha,
				colour_add(colour_scale(ubeta, cba), colour_scale(ualpha, cbb)))
			);
}

CubeMap *cubemap_load(const char *prefix)
{
	CubeMap *map;
	char filename[1024];

	map = malloc(sizeof(CubeMap));

	for (int i = 0; i < 6; i++)
	{
		sprintf(filename, "%s_%s.png", prefix, cube_direction_str[i]);
		map->texture[i] = texture_load_png(filename);
		if (map->texture[i] == NULL)
			return NULL;
	}
	return map;
}

Colour cubemap_colour(CubeMap *map, Vec3 d)
{
	Texture *texture = NULL;
	float u = 0.0, v = 0.0;
	double dx = fabs(d.x), dy = fabs(d.y), dz = fabs(d.z);
	if (dx >= dy && dx >= dz)
	{
		if (d.x >= 0)
		{
			u = (1.0 + d.z/d.x)/2.0;
			v = (1.0 + d.y/d.x)/2.0;
			texture = map->texture[POSITIVE_X];
		} else
		{
			u = 1.0 - (1.0 - d.z/d.x)/2.0;
			v = (1.0 - d.y/d.x)/2.0;
			texture = map->texture[NEGATIVE_X];
		}
	} else if (dy >= dz && dy >= dx)
	{
		if (d.y >= 0)
		{
			u = (1.0 + d.x/d.y)/2.0;
			v = 1.0 - (1.0 - d.z/d.y)/2.0;
			texture = map->texture[POSITIVE_Y];
		} else
		{
			u = (1.0 - d.x/d.y)/2.0;
			v = (1.0 + d.z/d.y)/2.0;
			texture = map->texture[NEGATIVE_Y];
		}
	} else if (dz >= dx && dz >= dy)
	{
		if (d.z >= 0)
		{
			u = 1.0 - (1.0 + d.x/d.z)/2.0;
			v = (1.0 + d.y/d.z)/2.0;
			texture = map->texture[POSITIVE_Z];
		} else
		{
			u = (1.0 - d.x/d.z)/2.0;
			v = (1.0 - d.y/d.z)/2.0;
			texture = map->texture[NEGATIVE_Z];
		}
	}

	return texture_texel(texture, u, v);
}
