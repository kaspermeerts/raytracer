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
	Texture *texture;
	Colour *buffer;
	const int width = 500, height = 972;

	if (argc < 2)
		return 1;

	texture = texture_load_png("rainbowdash.png");
	buffer = calloc(width * height, sizeof(Colour));
	for (int y = 0; y < height; y++)
	for (int x = 0; x < width; x++)
	{
		buffer[y*width + x] = texture_texel(texture, ((float)x)/width, ((float)y)/height);
	}
	out = fopen(argv[1], "wb");
	if (out == NULL)
	{
		printf("Error opening %s\n", argv[1]);
		free(texture);
		return 1;
	}
	ppm_write(buffer, width, height, out);
	return 0;
}
