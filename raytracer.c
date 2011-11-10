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

Sphere spheres[2] = {
{{0.0, 0.0, 0.0}, 1.0 },
{{2.0, 2.0, -1.0}, 2.0 }
};

int main(void)
{
	Vec3 camera = (Vec3){0.0, 0.0, 5.0};
	FILE *out;
	Colour *buffer;

	buffer = calloc(WIDTH*HEIGHT, sizeof(Colour));

	for (int j = 0; j < HEIGHT; j++)
	{
		for (int i = 0; i < WIDTH; i++)
		{
			Colour *c = &buffer[WIDTH*j + i];
			Vec3 d;
			float x, y, z, t1, t2;
			Ray r;

			x = -5 + 10.0 * i/(float) WIDTH;
			y = -5 + 10.0 * j/(float) HEIGHT;
			z = 3;

			d.x = x;
			d.y = y;
			d.z = z;

			r.origin = camera;
			r.direction = vec3_normalize(vec3_sub(d, camera));

#if 1
			r.origin = d;
			r.direction = (Vec3) {0, 0, -1};
#endif
			t1 = ray_sphere_intersect(r, spheres[0]);
			t2 = ray_sphere_intersect(r, spheres[1]);

			if (t1 > 0 && (t1 < t2 || t2 < 0))
			{
				c->r = 255;
				c->g = 0;
				c->b = 0;
			} else if (t2 > 0)
			{
				c->r = 0;
				c->g = 255;
				c->b = 0;
			}
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
