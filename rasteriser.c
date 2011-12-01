#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "colour.h"
#include "scene.h"
#include "ppm.h"
#include "raster.h"

static void tesselate_shape(Shape *shape)
{
	switch(shape->type)
	{
	case SHAPE_MESH:
		break;
	default:
		printf("Cannot tesselate shape \"%s\"\n", shape->name);
		break;
	}

	shape->type = SHAPE_MESH;
}

static Colour fragment_shader(Material *mat, Mesh *mesh,
		Triangle tri, float a, float b, float c)
{
	mat = mat;
	mesh = mesh;
	tri = tri;
	a = a;
	b = b;
	c = c;

	return mat->diffuse_colour;
}

static void raster_triangle(Raster *raster, Material *mat, Mesh *mesh,
		Triangle tri, int x0, int y0, float z0,
		int x1, int y1, float z1, int x2, int y2, float z2)
{
	int xmin, ymin, xmax, ymax;
	float fa, fb, fc;
	float fao, fbo, fco; /* Coefficients for offscreen point */

	xmin = MIN(x0, MIN(x1, x2));
	ymin = MIN(y0, MIN(y1, y2));
	xmax = MAX(x0, MAX(x1, x2));
	ymax = MAX(y0, MAX(y1, y2));

	fa = (y1 - y2)*x0 + (x2 - x1)*y0 + x1*y2 - x2*y1;
	fb = (y2 - y0)*x1 + (x0 - x2)*y1 + x2*y0 - x0*y2;
	fc = (y0 - y1)*x2 + (x1 - x0)*y2 + x0*y1 - x1*y0;

	fao = -(y1 - y2) + -(x2 - x1) + x1*y2 - x2*y1;
	fbo = -(y2 - y0) + -(x0 - x2) + x2*y0 - x0*y2;
	fco = -(y0 - y1) + -(x1 - x0) + x0*y1 - x1*y0;

	xmin = MAX(0, xmin);
	xmax = MIN(raster->width, xmax);

	ymin = MAX(0, ymin);
	ymax = MIN(raster->height, ymax);
	for (int y = ymin; y <= ymax; y++)
	{
		for (int x = xmin; x <= xmax; x++)
		{
			float a, b, c;
			Colour col;

			a = ((y1 - y2)*x + (x2 - x1)*y + x1*y2 - x2*y1)/fa;
			b = ((y2 - y0)*x + (x0 - x2)*y + x2*y0 - x0*y2)/fb;
			c = ((y0 - y1)*x + (x1 - x0)*y + x0*y1 - x1*y0)/fc;

			/*
			col = colour_add(colour_add(
					colour_scale(a, RED),
					colour_scale(b, GREEN)),
					colour_scale(c, BLUE));
			*/
			col = RED;
			if (a >= 0 && b >= 0 && c >= 0)
			{
				if (a > 0 || fa*fao > 0)
				if (b > 0 || fb*fbo > 0)
				if (c > 0 || fc*fco > 0)
				{
					float z;
					z = a*z0 + b*z1 + c*z2;
					if (raster_z_pixel(raster, x, y, z))
					{
						col = fragment_shader(mat, mesh, tri, a, b, c);
						raster_pixel(raster, x, y, col);
					}
				}
			}
		}
	}
}

static void rasterise_mesh(Raster *raster, Mesh *mesh, Material *mat, Mat4 mvp)
{
	for (int i = 0; i < mesh->num_triangles; i++)
	{
		Vec4 a4, b4, c4;
		Vec3 a, b, c;
		int x0, x1, x2, y0, y1, y2;
		float z0, z1, z2;
		int nx, ny;
		Triangle tri;

		tri = mesh->triangle[i];

		a4 = vec4_from_vec3(
		    	mesh->vertex[tri.vertex[0].vertex_index], 1);
		b4 = vec4_from_vec3(
		    	mesh->vertex[tri.vertex[1].vertex_index], 1);
		c4 = vec4_from_vec3(
				mesh->vertex[tri.vertex[2].vertex_index], 1);

		a4 = mat4_transform(mvp, a4);
		b4 = mat4_transform(mvp, b4);
		c4 = mat4_transform(mvp, c4);

		a = vec4_project(a4);
		b = vec4_project(b4);
		c = vec4_project(c4);

		nx = raster->width; ny = raster->height;
		x0 = nx/2*(a.x + 1) - 0.5;
		x1 = nx/2*(b.x + 1) - 0.5;
		x2 = nx/2*(c.x + 1) - 0.5;

		y0 = ny/2*(a.y + 1) - 0.5;
		y1 = ny/2*(b.y + 1) - 0.5;
		y2 = ny/2*(c.y + 1) - 0.5;

		z0 = a.z;
		z1 = b.z;
		z2 = c.z;

#if 0
		raster_line(raster, x0, y0, x1, y1);
		raster_line(raster, x1, y1, x2, y2);
		raster_line(raster, x2, y2, x0, y0);
#else
		raster_triangle(raster, mat, mesh, tri, x0, y0, z0, x1, y1, z1, x2, y2, z2);
#endif
	}
}

static void cam_proj_matrix(const Camera *cam, Mat4 proj, float znear, float zfar)
{
	float aspect = ((float) cam->width) / ((float) cam->height);
	mat4_perspective(proj, cam->fov*M_TWO_PI/360., aspect, znear, zfar);
}

static void cam_view_matrix(const Camera *cam, Mat4 view)
{
	Mat4 d;

	mat4_identity(view);
	/* Because the matrix transforms vertices, we have to give
	 * the inverse transformation */
	mat4_quaternion(d, quat_conjugate(cam->orientation));
	mat4_rmul(view, d);
	mat4_translate_vector(d, vec3_scale(-1, cam->position));
	mat4_rmul(view, d);
}

static void rasterise(Raster *raster)
{
	Mat4 proj, view, model, mvp;
	Surface *surface = scene->root;
	assert(surface->shape->type == SHAPE_MESH);
	Mesh *mesh = surface->shape->u.mesh;

	/* Projection */
	//mat4_ortho(proj, -1, 1, -1, 1, -1, 1);
	cam_proj_matrix(scene->camera, proj, -1, -100);
	/* View */
	cam_view_matrix(scene->camera, view);
	/* Model */
	mat4_copy(model, surface->model_to_world);

	mat4_identity(mvp);
	mat4_rmul(mvp, proj);
	mat4_rmul(mvp, view);
	mat4_rmul(mvp, model);

#if 0
	printf("Projection\n");
	mat4_print(proj);
	printf("\nView\n");
	mat4_print(view);
	printf("\nModel\n");
	mat4_print(model);
	printf("\nmvp\n");
	mat4_print(mvp);
#endif

	rasterise_mesh(raster, mesh, surface->material, mvp);
}

int main(int argc, char **argv)
{
	Sdl *sdl;
	Raster *raster;
	FILE *out;
	int width, height;

	if (argc < 2)
	{
		printf("Give an SDL file as an argument\n");
		return 1;
	}

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;
	width = scene->camera->width;
	height = scene->camera->height;

	for (int i = 0; i < sdl->num_shapes; i++)
		tesselate_shape(&sdl->shape[i]);

	raster = raster_new(width, height);
	rasterise(raster);

	out = fopen("raster.ppm", "wb");
	ppm_write(raster->buffer, raster->width, raster->height, out);
	fclose(out);
	raster_destroy(raster);

	return 0;
}
