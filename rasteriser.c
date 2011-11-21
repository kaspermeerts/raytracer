#include <stdio.h>
#include <stdlib.h>

#include "colour.h"
#include "scene.h"
#include "ppm.h"

#define WIDTH  512
#define HEIGHT 512

int main(int argc, char **argv)
{
	Sdl *sdl;
	Colour *buffer;
	FILE *out;

	if (argc < 2)
	{
		printf("Give an SDL file as an argument\n");
		return 1;
	}

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	buffer = calloc(WIDTH*HEIGHT, sizeof(Colour));

	out = fopen("raster.ppm", "wb");
	ppm_write(buffer, WIDTH, HEIGHT, out);
	fclose(out);

	return 0;
}
