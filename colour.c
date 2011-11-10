#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "colour.h"

bool write_image(Colour *buffer, int width, int height, FILE *fd)
{
	int i, j;
	float max;

	fprintf(fd, "P3\n");
	fprintf(fd, "%d %d\n", width, height);
	fprintf(fd, "%d\n", 255); /* XXX ? */

	max = -HUGE_VAL;
	for (i = 0; i < width*(height - 1); i++)
	{
		if (buffer[i].r > max)
			max = buffer[i].r;
		if (buffer[i].g > max)
			max = buffer[i].g;
		if (buffer[i].b > max)
			max = buffer[i].b;
	}
	max /= 255;
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			Colour c = buffer[width*(height - 1 - j) + i];
			fprintf(fd, "%u %u %u ", (int) (c.r/max), (int) (c.g/max),
					(int) (c.b/max));
		}
		fprintf(fd, "\n");
	}

	return true;
}
