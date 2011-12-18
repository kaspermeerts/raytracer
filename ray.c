#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "ray.h"

struct TriangleHit {
	double t;
	float a;
	float b;
	float c;
	Triangle triangle;
};

double drand(void)
{
	return rand()/((double) RAND_MAX);
}

static Ray cam_ray_internal(Camera *cam, int i, int j, float offx, float offy,
		double near)
{
	Ray r;
	float d, u, v;
	double bottom, left, width, height;
	const int nx = config->width, ny = config->height;

	width = 2*near*tan(cam->fov*M_TWO_PI/360./2.);
	left = -width/2;
	height = width * ny/(double) nx;
	bottom = -height/2;

	d = near;
	u = left +   width *(i + offx)/nx;
	v = bottom + height*(j + offy)/ny;
	r.origin = cam->position;
	r.direction = vec3_normalize(vec3_add(
			vec3_scale(-d, cam->w), vec3_add(
			vec3_scale( u, cam->u),
			vec3_scale( v, cam->v))));
	r.near = 0;
	r.far = HUGE_VAL;

	return r;
}

/* Fullscreen antialiasing. Ultra-slow. */
Ray camera_ray_aa(Camera *cam, int i, int j, int sample, double near)
{
	float offx, offy;
	int p, q;
	const float n = (float) config->aa_samples;

	p = sample % config->aa_samples;
	q = sample / config->aa_samples;
	offx = (p + drand()) / n;
	offy = (q + drand()) / n;

	return cam_ray_internal(cam, i, j, offx, offy, near);
}

/* The offset 0.5 traces the ray right through the center of the pixel. */
Ray camera_ray(Camera *cam, int i, int j, double near)
{
	return cam_ray_internal(cam, i, j, 0.5, 0.5, near);
}

/* Various intersection routines. The sphere and cylinder routines are the most
 * mature. */
static int ray_plane_intersect(Ray ray, Plane plane, float t[1], Vec3 normal[1])
{
	float test_t, alpha, beta, det;
	Vec3 a = plane.edge1, b = plane.edge2, d = ray.direction, o = ray.origin;
	Vec3 n, axn, bxn, pos;

	/* This is basically Cramer's rule with vector calculus */
	n = vec3_cross(a, b);
	test_t = -vec3_dot(o, n)/vec3_dot(d, n);
	pos = vec3_add(o, vec3_scale(test_t, d));

	axn = vec3_cross(a, n);
	bxn = vec3_cross(b, n);
	det = vec3_dot(a, bxn);

	alpha =  vec3_dot(pos, bxn)/det;
	beta  = -vec3_dot(pos, axn)/det;

	if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1)
		return 0;

	t[0] = test_t;
	if (vec3_dot(d, n) < 0)
		normal[0] = n;
	else
		normal[0] = vec3_scale(-1, n);

	return 1;
}

static int ray_disk_intersect(Ray ray, Disk disk, float t[1], Vec3 normal[1])
{
	double tt, xx, yy;
	Vec3 o = ray.origin, d = ray.direction;

	if (d.z == 0)
		return 0;

	tt = -o.z/d.z;
	xx = o.x + tt*d.x;
	yy = o.y + tt*d.y;
	if (SQUARE(xx) + SQUARE(yy) > SQUARE(disk.radius))
		return 0;

	t[0] = tt;
	if (o.z + ray.near*d.z >= 0)
		normal[0] = (Vec3) {0, 0,  1};
	else
		normal[0] = (Vec3) {0, 0, -1};

	return 1;
}

static int ray_sphere_intersect(Ray r, Sphere sph, float t[2], Vec3 normal[2])
{
	Vec3 v;
	float dd, vd, vv, discriminant;
	float radius = sph.radius;

	v = r.origin;
	vv = vec3_dot(v, v);
	vd = vec3_dot(v, r.direction);
	dd = vec3_dot(r.direction, r.direction);

	discriminant = vd*vd - dd*(vv - radius*radius);
	if (discriminant < 0)
		return 0;
	else if (discriminant == 0)
	{
		t[0] = -vd/dd;
		normal[0] = vec3_normalize(vec3_add(r.origin,
				vec3_scale(t[0], r.direction)));
		return 1;
	} else
	{
		t[0] = (-vd - sqrtf(discriminant))/dd;
		t[1] = (-vd + sqrtf(discriminant))/dd;

		normal[0] = vec3_add(r.origin, vec3_scale(t[0], r.direction));
		normal[1] = vec3_add(r.origin, vec3_scale(t[1], r.direction));
		return 2;
	}
}

static int ray_cylinder_intersect(Ray ray, Cylinder cyl, float t[2],
		Vec3 normal[2])
{
	const float height = cyl.height, radius = cyl.radius;
	const Vec3 o = ray.origin, d = ray.direction;
	float a, b, c, disc, z0, z1;
	bool capped = cyl.capped;

	a = SQUARE(d.x) + SQUARE(d.y);
	b = 2*(o.x*d.x + o.y*d.y);
	c = SQUARE(o.x) + SQUARE(o.y) - SQUARE(radius);

	/* If the projection of the ray on the XY plane doesn't cross the circle
	 * formed by the projection of the cylinder, it certainly never intersects
	 * the cylinder. */
	disc = b*b - 4*a*c;
	if (disc < 0)
		return 0;

	/* The preliminary intersection points. These might be too high or low. */
	t[0] = (-b-sqrtf(disc))/(2*a);
	normal[0].x = (o.x + t[0]*d.x)/radius;
	normal[0].y = (o.y + t[0]*d.y)/radius;
	normal[0].z = 0;
	t[1] = (-b+sqrtf(disc))/(2*a);
	normal[1].x = (o.x + t[1]*d.x)/radius;
	normal[1].y = (o.y + t[1]*d.y)/radius;
	normal[1].z = 0;

	/* The heights will determine whether we hit the actual cylinder, a cap or
	 * went straight over or under */
	z0 = o.z + t[0]*d.z;
	z1 = o.z + t[1]*d.z;
	if ((z0 < 0 && z1 < 0) || (z0 > height && z1 > height))
		return false; /* Over or under */
	if ((z0 < 0 && z1 > height) || (z0 > height && z1 < 0))
	{
		/* We went through the top and out the bottom (or vice-versa) without
		 * touching the sides */
		if (!capped)
			return false;

		/* The order of t[0] and t[1] is arbitrary, there's no guarantee t[0]
		 * will be the closest intersection point. */
		t[0] = -o.z/d.z;
		normal[0] = (Vec3) {0, 0, -1};
		t[1] = (height - o.z)/d.z;
		normal[1] = (Vec3) {0, 0, 1};
	}
	else if (z0 >= 0 && z0 <= height && z1 > height)
	{
		/* t[0] is correct, yet t[1] is too high and will actually hit the cap,
		 * if one exists. */
		if (capped)
		{
			t[1] = (height - o.z)/d.z;
			normal[1] = (Vec3) {0, 0, 1};
		} else
		{
			t[1] = t[0];
			normal[1] = normal[0];
		}
	}
	else if (z0 >= 0 && z0 <= height && z1 < 0)
	{
		/* t[0] is correct and t[1] is too low. */
		if (capped)
		{
			t[1] = -o.z/d.z;
			normal[1] = (Vec3) {0, 0, -1};
		}
		else
		{
			t[1] = t[0];
			normal[1] = normal[0];
		}
	}
	else if (z1 >= 0 && z1 <= height && z0 > height)
	{
		/* t[1] is correct, yet t[0] is too high and will actually hit the cap,
		 * if one exists. */
		if (capped)
		{
			t[0] = (height - o.z)/d.z;
			normal[0] = (Vec3) {0, 0, 1};
		}
		else
		{
			t[0] = t[1];
			normal[0] = normal[1];
		}
	}
	else if (z1 >= 0 && z1 <= height && z0 < 0)
	{
		/* t[1] is correct and t[0] is too low. */
		if (capped)
		{
			t[0] = -o.z/d.z;
			normal[0] = (Vec3) {0, 0, -1};
		}
		else
		{
			t[0] = t[1];
			normal[0] = normal[1];
		}
	}
	else if (z0 >= 0 && z0 <= height && z1 >= 0 && z1 <= height)
	{
		/* Nothing left to be fixed */
	}
	else
	{
		printf("Unhandled case: %g %g %g %g\n", t[0], t[1], z0, z1);
		__asm("int3");
		assert("Unhandled case" == NULL);
	}

	return 2;
}

static int ray_cone_intersect(Ray ray, Cone cone, float t[2], Vec3 normal[2])
{
	float dx, dy, dz, ox, oy, oz, R, h;
	float a, b, c, disc;
	float z0, z1;

	dx = ray.direction.x;
	dy = ray.direction.y;
	dz = ray.direction.z;

	ox = ray.origin.x;
	oy = ray.origin.y;
	oz = ray.origin.z;

	R = cone.radius;
	h = cone.height;

	a = SQUARE(dx) + SQUARE(dy) - SQUARE(R/h*dz);
	b = 2*(ox*dx + oy*dy + SQUARE(R/h)*(- oz*dz + h*dz));
	c = SQUARE(ox) + SQUARE(oy) - SQUARE(R/h)*(SQUARE(h) - 2*h*oz + SQUARE(oz));

	disc = b*b - 4*a*c;
	if (disc < 0)
		return 0;

	t[0] = (-b - sqrtf(disc))/(2*a);
	t[1] = (-b + sqrtf(disc))/(2*a);

	z0 = oz + t[0]*dz;
	z1 = oz + t[1]*dz;

	if ((z0 > h || z0 < 0) && (z1 > h || z1 < 0))
		return false;
	else if (z0 > h || z0 < 0)
		t[0] = t[1];
	else if (z1 > h || z1 < 0)
		t[1] = t[0];

	normal[0].x = h/sqrtf(h*h+R*R)*(ox+t[0]*dx)/
			sqrtf(SQUARE(ox+t[0]*dx) + SQUARE(oy+t[0]*dy));
	normal[0].y = h/sqrtf(h*h+R*R)*(oy+t[0]*dy)/
			sqrtf(SQUARE(ox+t[0]*dx) + SQUARE(oy+t[0]*dy));
	normal[0].z = R/sqrtf(h*h+R*R);

	normal[1].x = h/sqrtf(h*h+R*R)*(ox+t[1]*dx)/
			sqrtf(SQUARE(ox+t[1]*dx) + SQUARE(oy+t[1]*dy));
	normal[1].y = h/sqrtf(h*h+R*R)*(oy+t[1]*dy)/
			sqrtf(SQUARE(ox+t[1]*dx) + SQUARE(oy+t[1]*dy));
	normal[1].z = R/sqrtf(h*h+R*R);

	return 1;
}

/* This is basically Cramer's rule, but with vector calculus. */
static bool ray_triangle_intersect(Ray ray, Vec3 u, Vec3 v, Vec3 w,
		struct TriangleHit *tri_hit)
{
	Vec3 edge1, edge2, tvec, pvec, qvec;
	float det;
	float aa, bb, cc;

	edge1 = vec3_sub(v, u);
	edge2 = vec3_sub(w, u);

	pvec = vec3_cross(ray.direction, edge2);
	det = vec3_dot(edge1, pvec);

	tvec = vec3_sub(ray.origin, u);
	bb = vec3_dot(tvec, pvec) / det;
	if (bb < 0.0 || bb > 1.0)
		return false;

	qvec = vec3_cross(tvec, edge1);
	cc = vec3_dot(ray.direction, qvec) / det;
	if (cc < 0.0 || cc + bb > 1.0)
		return false;

	aa = 1.0 - bb - cc;

	tri_hit->a = aa;
	tri_hit->b = bb;
	tri_hit->c = cc;
	tri_hit->t = vec3_dot(edge2, qvec) / det;

	return true;
}

static bool ray_kd_leaf_intersect(Ray ray, const Vec3 *vertex_list,
		const KdNode *leaf,	struct TriangleHit *hit)
{

	struct TriangleHit final_hit;
	final_hit.t = HUGE_VAL;
	for (int i = 0; i < leaf->num_triangles; i++)
	{
		struct TriangleHit nhit;
		Triangle tri = leaf->triangle[i];
		Vec3 u = vertex_list[tri.vertex_index[0]];
		Vec3 v = vertex_list[tri.vertex_index[1]];
		Vec3 w = vertex_list[tri.vertex_index[2]];

		if (ray_triangle_intersect(ray, u, v, w, &nhit))
		{
			if (nhit.t >= ray.near && nhit.t <= final_hit.t && nhit.t <= ray.far)
			{
				final_hit = nhit;
				final_hit.triangle = tri;
			}
		}
	}
	if (final_hit.t < HUGE_VAL)
	{
		*hit = final_hit;
		return true;
	}
	else
		return false;
}

static bool ray_kd_tree_intersect(Ray ray, const Vec3 *vertex_list,
		const KdNode *node, struct TriangleHit *hit)
{
	const Vec3 plane_normal[3] =
			{(Vec3) {1, 0, 0}, (Vec3) {0, 1, 0}, (Vec3) {0, 0, 1}};
	KdNode *node_near, *node_far;
	struct TriangleHit hit_near, hit_far;
	bool did_near, did_far;
	Ray ray_near = ray, ray_far = ray;
	double clip_t;

	/* In a leaf we have to check all triangles */
	if (node->leaf)
		return ray_kd_leaf_intersect(ray, vertex_list, node, hit);

	switch(node->axis)
	{
	case X_AXIS:
		clip_t = (node->location - ray.origin.x)/ray.direction.x;
		break;
	case Y_AXIS:
		clip_t = (node->location - ray.origin.y)/ray.direction.y;
		break;
	case Z_AXIS:
		clip_t = (node->location - ray.origin.z)/ray.direction.z;
		break;
	default:
		printf("Invalid axis %d\n", node->axis);
		exit(1);
		break;
	}

	if (vec3_dot(ray.direction, plane_normal[node->axis]) > 0.0)
	{
		node_near = node->left;
		node_far = node->right;
	} else
	{
		node_near = node->right;
		node_far = node->left;
	}

	/* The major performance improvement from using kd-trees
	 * Don't check a branch of a tree if the ray can't possibly intersect it */
	if (clip_t > ray.far)
		return ray_kd_tree_intersect(ray, vertex_list, node_near, hit);
	if (clip_t < ray.near)
		return ray_kd_tree_intersect(ray, vertex_list, node_far, hit);

	/* Split the ray in twain */
	ray_near.near = ray.near; ray_near.far = clip_t;
	ray_far.near  = clip_t;   ray_far.far  = ray.far;

	/* The invariant of a kd-tree is that every point in the near node will
	 * be closer than any point in the far node. So we start by checking the
	 * near node and if we find an intersection inside it, we don't check the
	 * far node anymore as it can't possible contain a closer intersection. */
	did_near = ray_kd_tree_intersect(ray_near, vertex_list, node_near, &hit_near);
	/* The test (hit_near.t < clip_t) is important, as it is possible a
	 * primitive in the far node will intersect closer than this primitive,
	 * which lies only partially in the near node. */
	if (did_near && hit_near.t <= clip_t)
	{
		*hit = hit_near;
		return true;
	}

	did_far = ray_kd_tree_intersect(ray_far, vertex_list, node_far, &hit_far);
	*hit = hit_far;
	return did_far;
}

static int ray_mesh_intersect(Ray ray, const Mesh *mesh, float *t, Vec3 *normal)
{
	struct TriangleHit tri_hit;

	if (ray_kd_tree_intersect(ray, mesh->vertex, mesh->kd_tree, &tri_hit))
	{
		Triangle tri = tri_hit.triangle;
		*t = tri_hit.t;
		*normal = vec3_add(vec3_add(
				vec3_scale(tri_hit.a, mesh->normal[tri.normal_index[0]]),
				vec3_scale(tri_hit.b, mesh->normal[tri.normal_index[1]])),
				vec3_scale(tri_hit.c, mesh->normal[tri.normal_index[2]]));
		return 1;
	}

	return 0;

}

static bool ray_surface_intersect(Ray ray, const Surface *surf, Hit *hit)
{
	float ts[2] = {-HUGE_VAL, -HUGE_VAL}, t;
	Vec3 tnormals[2] = {{0,0,0},{0,0,0}}, tnormal;
	Ray tray;
	Mat4 normal_matrix;
	Shape *shape = surf->shape;
	int hits;

	mat4_copy(normal_matrix, surf->world_to_model);
	mat4_transpose(normal_matrix);

	tray.origin = mat4_transform3_homo(surf->world_to_model, ray.origin);
	tray.direction = mat4_transform3_hetero(surf->world_to_model, ray.direction);
	tray.near = ray.near;
	tray.far = ray.far;

	switch(shape->type)
	{
	case SHAPE_PLANE:
		hits = ray_plane_intersect(tray, shape->u.plane, ts, tnormals);
		break;
	case SHAPE_DISK:
		hits = ray_disk_intersect(tray, shape->u.disk, ts, tnormals);
		break;
	case SHAPE_SPHERE:
		hits = ray_sphere_intersect(tray, shape->u.sphere, ts, tnormals);
		break;
	case SHAPE_CYLINDER:
		hits = ray_cylinder_intersect(tray, shape->u.cylinder, ts, tnormals);
		break;
	case SHAPE_CONE:
		hits = ray_cone_intersect(tray, shape->u.cone, ts, tnormals);
		break;
	case SHAPE_MESH:
		hits = ray_mesh_intersect(tray, shape->u.mesh, ts, tnormals);
		break;
	default:
		printf("Unknown shape\n");
		return false;
		break;
	}

	/* We're looking for the smallest hit that is between the near and far
	 * planes of the ray. */
	if (hits == 0)
		return false;
	if (hits == 1)
	{
		if (ts[0] < ray.near || ts[0] > ray.far)
			return false;

		t = ts[0];
		tnormal = tnormals[0];
	} else if (hits == 2)
	{
		bool t0_ok = ts[0] >= ray.near && ts[0] <= ray.far;
		bool t1_ok = ts[1] >= ray.near && ts[1] <= ray.far;

		     if (!t0_ok && !t1_ok)
		{
			return false;
		}
		else if (t0_ok && !t1_ok)
		{
			t = ts[0];
			tnormal = tnormals[0];
		}
		else if (!t0_ok && t1_ok)
		{
			t = ts[1];
			tnormal = tnormals[1];
		}
		else
		{
			if (ts[0] < ts[1])
			{
				t = ts[0];
				tnormal = tnormals[0];
			} else
			{
				t = ts[1];
				tnormal = tnormals[1];
			}
		}
	} else
	{
		printf("General t finding code unimplemented\n");
		return false;
	}

	hit->t = t;
	hit->position = vec3_add(ray.origin, vec3_scale(t, ray.direction));
	hit->normal = vec3_normalize(mat4_transform3_hetero(normal_matrix, tnormal));
	return true;
}

static bool ray_bbox_test(Ray ray, BBox bbox, Ray *bray)
{
	float xmin = bbox.xmin, xmax = bbox.xmax;
	float ymin = bbox.ymin, ymax = bbox.ymax;
	float zmin = bbox.zmin, zmax = bbox.zmax;
	float txmin, txmax, tymin, tymax, tzmin, tzmax;
	float xd = ray.direction.x, yd = ray.direction.y, zd = ray.direction.z;
	float xe = ray.origin.x,    ye = ray.origin.y,    ze = ray.origin.z;
	float xa, ya, za;

	xa = 1.0/xd;
	if (xa >= 0)
	{
		txmin = (xmin - xe)*xa;
		txmax = (xmax - xe)*xa;
	} else
	{
		txmin = (xmax - xe)*xa;
		txmax = (xmin - xe)*xa;
	}

	ya = 1.0/yd;
	if (ya >= 0)
	{
		tymin = (ymin - ye)*ya;
		tymax = (ymax - ye)*ya;
	} else
	{
		tymin = (ymax - ye)*ya;
		tymax = (ymin - ye)*ya;
	}

	za = 1.0/zd;
	if (za >= 0)
	{
		tzmin = (zmin - ze)*za;
		tzmax = (zmax - ze)*za;
	} else
	{
		tzmin = (zmax - ze)*za;
		tzmax = (zmin - ze)*za;
	}

	assert(txmin <= txmax);
	assert(tymin <= tymax);
	assert(tzmin <= tzmax);

	if ((txmin > tymax) || (tymin > txmax))
		return false;

	if ((tymin > tzmax) || (tzmin > tymax))
		return false;

	if ((tzmin > txmax) || (txmin > tzmax))
		return false;

	bray->origin = ray.origin;
	bray->direction = ray.direction;
	bray->near = MAX(txmin, MAX(tymin, MAX(tzmin, ray.near)));
	bray->far = MIN(txmax, MIN(tymax, MIN(tzmax, ray.far)));
	return true;
}

bool ray_intersect(Ray ray, Hit *hit)
{
	Hit test_hit;
	Surface *surface;

	hit->surface = NULL;
	hit->t = HUGE_VAL;

	for (surface = scene->root; surface; surface = surface->next)
	{
		Ray bray;
		/* Test the surface's bounding box and clip the ray if necessary */
		if (!ray_bbox_test(ray, surface->bbox, &bray))
			continue;

		test_hit.surface = surface;
		if (ray_surface_intersect(bray, surface, &test_hit))
		{
			if (hit->surface == NULL || test_hit.t < hit->t)
			{
				*hit = test_hit;
			}
		}
	}

	if (hit->surface != NULL)
		return true;
	else
		return false;
}
