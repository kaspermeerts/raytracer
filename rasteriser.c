#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "glm.h"
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

static void raster_triangle(Raster *raster, int x0, int y0, int x1, int y1, int x2, int y2)
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

	for (int y = ymin; y <= ymax; y++)
	{
		for (int x = xmin; x <= xmax; x++)
		{
			float a, b, c;
			Colour col;

			a = ((y1 - y2)*x + (x2 - x1)*y + x1*y2 - x2*y1)/fa;
			b = ((y2 - y0)*x + (x0 - x2)*y + x2*y0 - x0*y2)/fb;
			c = ((y0 - y1)*x + (x1 - x0)*y + x0*y1 - x1*y0)/fc;

			col = colour_add(colour_add(
					colour_scale(a, RED),
					colour_scale(b, GREEN)),
					colour_scale(c, BLUE));

			if (a >= 0 && b >= 0 && c >= 0)
			{
				if (a > 0 || fa*fao > 0)
				if (b > 0 || fb*fbo > 0)
				if (c > 0 || fc*fco > 0)
					raster_pixel(raster, x, y, col);
			}
		}
	}
}

static void rasterise_mesh(Raster *raster, Mesh *mesh, Matrix *mvp)
{
	for (int i = 0; i < mesh->num_triangles; i++)
	{
		Vec4 a, b, c;
		int x0, x1, x2, y0, y1, y2;
		int nx, ny;

		a = vec4_from_vec3(
				mesh->vertex[mesh->triangle[i].vertex[0].vertex_index], 1);
		b = vec4_from_vec3(
				mesh->vertex[mesh->triangle[i].vertex[1].vertex_index], 1);
		c = vec4_from_vec3(
				mesh->vertex[mesh->triangle[i].vertex[2].vertex_index], 1);

		a = glmTransformVector(mvp, a);
		b = glmTransformVector(mvp, b);
		c = glmTransformVector(mvp, c);

		a = vec4_project(a);
		b = vec4_project(b);
		c = vec4_project(c);

		nx = raster->width; ny = raster->height;
		x0 = nx/2*(a.x + 1) - 0.5;
		x1 = nx/2*(b.x + 1) - 0.5;
		x2 = nx/2*(c.x + 1) - 0.5;

		y0 = ny/2*(a.y + 1) - 0.5;
		y1 = ny/2*(b.y + 1) - 0.5;
		y2 = ny/2*(c.y + 1) - 0.5;

#if 0
		raster_line(raster, x0, y0, x1, y1);
		raster_line(raster, x1, y1, x2, y2);
		raster_line(raster, x2, y2, x0, y0);
#else
		raster_triangle(raster, x0, y0, x1, y1, x2, y2);
#endif
	}
}

static void cam_proj_matrix(const Camera *cam, Matrix *proj, float znear, float zfar)
{
	float aspect = ((float) cam->width) / ((float) cam->height);
	printf("%g %g\n", cam->fov, aspect);
	glmPerspective(proj, cam->fov*M_TWO_PI/360., aspect, znear, zfar);
}

static void cam_view_matrix(const Camera *cam, Matrix *view)
{
	glmLoadIdentity(view);
	/* Because the matrix transforms vertices, we have to give
	 * the inverse transformation */
	glmMultQuaternion(view, quat_conjugate(cam->orientation));
	glmTranslateVector(view, vec3_scale(-1, cam->position));
}

static void rasterise(Raster *raster, Scene *scene)
{
	Matrix *mvp, *projection, *view, *model;
	double proj_m[16], view_m[16], model_m[16];
	Surface *surface = scene->root;
	assert(surface->shape->type == SHAPE_MESH);
	Mesh *mesh = surface->shape->u.mesh;

	projection = glmNewMatrixStack();
	view = glmNewMatrixStack();
	model = glmNewMatrixStack();
	mvp = glmNewMatrixStack();

	/* Projection */
	glmLoadIdentity(projection);
	//glmOrtho(projection, -1, 1, -1, 1, -1, 1);
	cam_proj_matrix(scene->camera, projection, 0, 100e9);
	/* View */
	cam_view_matrix(scene->camera, view);
	/* Model */
	glmLoadMatrix(model, surface->model_to_world);

	glmSaveMatrix(projection, proj_m);
	glmSaveMatrix(view, view_m);
	glmSaveMatrix(model, model_m);

	glmLoadIdentity(mvp);
	glmMultMatrix(mvp, proj_m);
	glmMultMatrix(mvp, view_m);
	glmMultMatrix(mvp, model_m);

	printf("Projection\n");
	glmPrintMatrix(projection);
	printf("\nView\n");
	glmPrintMatrix(view);
	printf("\nModel\n");
	glmPrintMatrix(model);
	printf("\nmvp\n");
	glmPrintMatrix(mvp);

	rasterise_mesh(raster, mesh, mvp);
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
	width = sdl->scene.camera->width;
	height = sdl->scene.camera->height;

	for (int i = 0; i < sdl->num_shapes; i++)
		tesselate_shape(&sdl->shape[i]);

	raster = raster_new(width, height);
	rasterise(raster, &sdl->scene);

	out = fopen("raster.ppm", "wb");
	ppm_write(raster->buffer, raster->width, raster->height, out);
	fclose(out);
	raster_destroy(raster);

	return 0;
}
