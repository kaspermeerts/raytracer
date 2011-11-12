#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cgmath.h"
#include "colour.h"
#include "scene.h"
#include "ray.h"

/* TODO: Put in config structure */
const int WIDTH = 1000;
const int HEIGHT = 1000;

Sdl *sdl;

int main(int argc, char **argv)
{
	Vec3 camera = (Vec3){0.0, 5.0, 1.0};
	FILE *out;
	Colour *buffer;

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	buffer = calloc(WIDTH*HEIGHT, sizeof(Colour));

	for (int j = 0; j < HEIGHT; j++)
	{
		for (int i = 0; i < WIDTH; i++)
		{
			Colour *c = &buffer[WIDTH*j + i];
			Vec3 d;
			float x, y, z, t1;
			Ray r;

			x = -5 + 10.0 * i/(float) WIDTH;
			y = 3;
			z = -5 + 10.0 * j/(float) HEIGHT;

			d.x = x;
			d.y = y;
			d.z = z;

			r.origin = camera;
			r.direction = vec3_normalize(vec3_sub(d, camera));

#if 0
			r.origin = d;
			r.direction = (Vec3) {0, 0, -1};
#endif
			t1 = ray_cylinder_intersect(r, sdl->scene.graph.shape->u.cylinder.height, sdl->scene.graph.shape->u.cylinder.radius);

			if (t1 > 0)
				c->r = c->g = c->b = 1;
			else
				c->r = c->g = c->b = 0;
		}
	}

	out = fopen("test.ppm", "w");
	write_image(buffer, WIDTH, HEIGHT, out);
	free(buffer);
	fclose(out);

	return 0;
}
