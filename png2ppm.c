#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "pnglite/pnglite.h"

#include "colour.h"
#include "ppm.h"

int main(int argc, char **argv)
{
	FILE *out;
	unsigned char *data;
	Colour *buffer;
	png_t png;

	if (argc < 3)
		return 1;

	png_init(NULL, NULL);
	if (png_open_file_read(&png, argv[1]) != PNG_NO_ERROR)
		return 1;

	data = calloc(png.width * png.height * png.bpp, 1);
	if (png_get_data(&png, data) != PNG_NO_ERROR)
	{
		printf("Couldn't get data from PNG\n");
		png_close_file(&png);
		return 1;
	}
	png_close_file(&png);

	buffer = calloc(png.width * png.height, sizeof(Colour));
	for (int i = 0; i < (signed) png.height; i++)
	for (int j = 0; j < (signed) png.width; j++)
	{
		int ii = png.height - 1 - i;
		buffer[png.width*ii + j].r = data[4*(png.width*i + j) + 0] / 255.;
		buffer[png.width*ii + j].g = data[4*(png.width*i + j) + 1] / 255.;
		buffer[png.width*ii + j].b = data[4*(png.width*i + j) + 2] / 255.;
		buffer[png.width*ii + j].a = data[4*(png.width*i + j) + 3] / 255.;
	}
	free(data);

	if ((out = fopen(argv[2], "wb")) == NULL)
	{
		free(buffer);
		printf("Couldn't open %s\n", argv[2]);
		return 1;
	}
	ppm_write(buffer, png.width, png.height, out);
	free(buffer);
	fclose(out);

	return 0;
}
