#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "pnglite/pnglite.h"

#include "colour.h"
#include "ppm.h"
#include "texture.h"

int main(int argc, char **argv)
{
	FILE *out;
	CubeMap *map;

	if (argc < 2)
		return 1;

	map = cubemap_load("cubemap/deadmeat_skymorning");
	out = fopen(argv[1], "wb");
	if (out == NULL)
	{
		printf("Error opening %s\n", argv[1]);
		free(map);
		return 1;
	}
	ppm_write(map->buffer[0], map->width, map->height, out);
	return 0;
}
