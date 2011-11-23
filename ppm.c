#include <stdbool.h>
#include <stdio.h>

#include "cgmath.h"
#include "colour.h"
#include "ppm.h"

bool ppm_write(Colour *buffer, int width, int height, FILE *fd)
{
	int i, j;
	size_t dummy;

	fprintf(fd, "P6\n");
	fprintf(fd, "%d %d\n", width, height);
	fprintf(fd, "%d\n", 255);

	for (j = 0; j < height; j++)
	{
	for (i = 0; i < width; i++)
	{
		Colour c = buffer[width*(height - 1 - j) + i];
		unsigned char out[3];
		out[0] = CLAMP(c.r * 255, 0, 255);
		out[1] = CLAMP(c.g * 255, 0, 255);
		out[2] = CLAMP(c.b * 255, 0, 255);
		dummy = fwrite(out, 1, 3, fd);
	}
	}

	return true;
}
