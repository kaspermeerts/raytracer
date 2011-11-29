#include <assert.h>
#include <math.h>
#include "ray.h"

Ray camera_ray(Camera *cam, int nx, int ny, int i, int j, double near)
{
	float d, u, v;
	Ray r;
	double bottom, left, width, height;

	width = near*tan(cam->fov*M_TWO_PI/360.);
	left = -width/2;
	height = width * ny/(double) nx;
	bottom = -height/2;

	d = near;
	u = left + width*(i + 0.5)/nx;
	v = bottom + height*(j + 0.5)/ny;
	r.origin = cam->position;
	r.direction = vec3_normalize(vec3_add(
			vec3_scale(-d, cam->w), vec3_add(
			vec3_scale( u, cam->u),
			vec3_scale( v, cam->v))));

	return r;
}

static bool ray_sphere_intersect(Ray r, Sphere sph, float t[2])
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
		return false;

	t[0] = (-vd - sqrtf(discriminant))/dd;
	t[1] = (-vd + sqrtf(discriminant))/dd;

	return true;
}

static bool ray_cylinder_intersect(Ray ray, Cylinder cyl, float t[2],
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
		return false;

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

		/* The order of t[0] and t[1] is arbitrary, there's no guarantee t[0] is the
		 * closest intersection point. */
		t[0] = -o.z/d.z;
		normal[0] = (Vec3) {0, 0, -1};
		t[1] = (height - o.z)/d.z;
		normal[1] = (Vec3) {0, 0, 1};
	}
	else if (z0 > 0 && z0 < height && z1 > height)
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
	else if (z0 > 0 && z0 < height && z1 < 0)
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
	else if (z1 > 0 && z1 < height && z0 > height)
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
	else if (z1 > 0 && z1 < height && z0 < 0)
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
	else if (z0 > 0 && z0 < height && z1 > 0 && z1 < height)
	{
		/* Nothing left to be fixed */
	}
	else
	{
		printf("Unhandled case: %g %g %g %g\n", t[0], t[1], z0, z1);
		__asm("int3");
		assert("Unhandled case" == NULL);
	}

	return true;
}

static bool ray_cone_intersect(Ray ray, Cone cone, float t[2], Vec3 normal[2])
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
		return false;

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

	return true;
}

static bool ray_triangle_intersect(Ray ray, Vec3 u, Vec3 v, Vec3 w, float *t, float *a, float *b, float *c)
{
	Vec3 edge1, edge2, tvec, pvec, qvec;
	float det;

	edge1 = vec3_sub(v, u);
	edge2 = vec3_sub(w, u);

	pvec = vec3_cross(ray.direction, edge2);
	det = vec3_dot(edge1, pvec);

	//assert(det > -0.0001 && det < 0.0001);

	tvec = vec3_sub(ray.origin, u);
	*b = vec3_dot(tvec, pvec) / det;
	if (*b < 0.0 || *b > 1.0)
		return false;

	qvec = vec3_cross(tvec, edge1);
	*c = vec3_dot(ray.direction, qvec) / det;
	if (*c < 0.0 || *c + *b > 1.0)
		return false;

	*t = vec3_dot(edge2, qvec) / det;
	*a = 1.0 - *b - *c;

	return true;
}

static bool ray_mesh_intersect(Ray ray, Mesh *mesh, float *t, Vec3 *normal)
{
	float tt;
	float a, b, c;

	*t = HUGE_VAL;
	for (int i = 0; i < mesh->num_triangles; i++)
	{
		Vec3 u, v, w;
		Triangle tri = mesh->triangle[i];

		u = mesh->vertex[tri.vertex[0].vertex_index];
		v = mesh->vertex[tri.vertex[1].vertex_index];
		w = mesh->vertex[tri.vertex[2].vertex_index];

		if (ray_triangle_intersect(ray, u, v, w, &tt, &a, &b, &c) && tt < *t)
		{
			*t = tt;
			*normal = vec3_add(vec3_add(
					vec3_scale(a, mesh->normal[tri.vertex[0].normal_index]),
					vec3_scale(b, mesh->normal[tri.vertex[1].normal_index])),
					vec3_scale(c, mesh->normal[tri.vertex[2].normal_index]));
		}
	}
	if (*t < HUGE_VAL)
		return true;
	return false;
}


static bool ray_surface_intersect(Ray ray, Surface *surf, Hit *hit)
{
	float t[2] = {-HUGE_VAL, -HUGE_VAL};
	Vec3 normal[2];
	bool intersect;

	switch(surf->shape->type)
	{
	case SHAPE_SPHERE:
		intersect = ray_sphere_intersect(ray, surf->shape->u.sphere, t);
		if (intersect)
		{
			if (t[0] < 0 && t[1] < 0)
			{
				intersect = false;
				break;
			}
			else if (t[0] > 0 && t[1] < 0)
				hit->t = t[0];
			else if (t[1] > 0 && t[0] < 0)
				hit->t = t[1];
			else
				hit->t = MIN(t[0], t[1]);

			hit->position = vec3_add(ray.origin,
					vec3_scale(hit->t, ray.direction));
			hit->normal = vec3_normalize(hit->position);
			return true;
		}
		break;
	case SHAPE_CYLINDER:
		if (ray_cylinder_intersect(ray, surf->shape->u.cylinder, t, normal))
		{
			if (t[0] < t[1])
			{
				hit->t = t[0];
				hit->normal = normal[0];
			} else
			{
				hit->t = t[1];
				hit->normal = normal[1];
			}
			hit->position = vec3_add(ray.origin,
					vec3_scale(hit->t, ray.direction));
			return true;
		}
		break;
	case SHAPE_CONE:
		if (ray_cone_intersect(ray, surf->shape->u.cone, t, normal))
		{
			if (t[0] < t[1])
			{
				hit->t = t[0];
				hit->normal = normal[0];
			}
			else
			{
				hit->t = t[1];
				hit->normal = normal[1];
			}
			hit->position = vec3_add(ray.origin,
					vec3_scale(hit->t, ray.direction));
			return true;
		}
		break;
	case SHAPE_MESH:
		if (ray_mesh_intersect(ray, surf->shape->u.mesh, t, normal))
		{
			hit->t = t[0];
			hit->normal = normal[0];
			hit->position = vec3_add(ray.origin, vec3_scale(hit->t, ray.direction));
			return true;
		}
		break;
	default:
		assert("Unknown shape" == 0);
		return false;
		break;
	}

	return false;
}

bool ray_intersect(Ray ray, Scene *scene, Hit *hit)
{
	Hit testhit;
	Surface *surface = scene->root;

	hit->surface = NULL;

	while (surface)
	{
		testhit.surface = surface;
		if (ray_surface_intersect(ray, surface, &testhit))
		{
			if (hit->surface == NULL || testhit.t < hit->t)
			{
				*hit = testhit;
			}
		}
		surface = surface->next;
	}

	if (hit->surface != NULL)
		return true;
	else
		return false;
}

/***********
 * Shading *
 ***********/

static Colour hit_colour(Hit *hit, Vec3 cam_pos, int n, Light **lights,
		Material *mat)
{
	Colour total;

	total = BLACK;
	for (int i = 0; i < n; i++)
	{
		Light *light = lights[i];
		Vec3 cam_dir, light_dir;
		Colour new;

		cam_dir = vec3_normalize(vec3_sub(cam_pos, hit->position));
		light_dir = vec3_normalize(vec3_sub(light->position, hit->position));

		new = light_mat_colour(light, mat, cam_dir, light_dir, hit->normal);
		total = colour_add(total, new);
	}

	return total;
}

Colour ray_colour(Ray ray, Scene *scene, int ttl)
{
	Hit hit;
	bool ray_hit;
	Material *mat;

	/* Early exit */
	if (ttl <= 0)
		return scene->background;

	ray_hit = ray_intersect(ray, scene, &hit);

	if (!ray_hit)
		return scene->background;

	mat = hit.surface->material;

	return hit_colour(&hit, scene->camera->position, scene->num_lights,
			scene->light, mat);
}
