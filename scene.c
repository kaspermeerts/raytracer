#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "scene.h"

static Vec3 parse_vec3(const char *string)
{
	assert(string != NULL);
	Vec3 v;
	char *end;

	v.x = strtof(string, &end);
	v.y = strtof(end, &end);
	v.z = strtof(end, &end);
printf("%g %g %g\n", v.x, v.y, v.z);
	return v;
}

static double parse_double(const char *string)
{
	assert(string != NULL);
	char *end;

	return strtod(string, &end);
}

static Colour parse_colour(const char *string)
{
	assert(string != NULL);
	Colour c;
	char *end;

	c.r = strtof(string, &end);
	c.g = strtof(end, &end);
	c.b = strtof(end, &end);

printf("%g %g %g\n", c.r, c.g, c.b);
	return c;
}

static char *strdup(const char *string)
{
	assert(string != NULL);
	char *new;
	size_t len;

	len = strlen(string) + 1;
	new = malloc(len);
	memcpy(new, string, len);
	new[len] = '\0';

	return new;
}

static bool import_cameras(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;

	sdl->num_cameras = n;
	sdl->camera = calloc(n, sizeof(Camera));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node; 
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Camera *cam = &sdl->camera[i];
		assert(strcmp(cur_node->name, "Camera") == 0);

		cam->position = parse_vec3(xmlGetProp(cur_node, "position"));
		cam->direction = parse_vec3(xmlGetProp(cur_node, "direction"));
		cam->up = parse_vec3(xmlGetProp(cur_node, "up"));
		cam->fov = parse_double(xmlGetProp(cur_node, "fovy"));
		cam->name = strdup(xmlGetProp(cur_node, "name"));
	}
	assert(i == n);

	return true;
}

static bool import_lights(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;

	sdl->num_lights = n;
	sdl->light = calloc(n, sizeof(Light));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node;
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Light *light = &sdl->light[i];

		if (strcmp(cur_node->name , "DirectionalLight") == 0)
		{
			light->type = LIGHT_DIRECTIONAL;
			light->direction = parse_vec3(xmlGetProp(cur_node, "direction"));
		} else if (strcmp(cur_node->name, "PointLight") == 0)
		{
			light->type = LIGHT_POINT;
			light->position = parse_vec3(xmlGetProp(cur_node, "position"));
		} else if (strcmp(cur_node->name, "SpotLight") == 0)
		{
			light->type = LIGHT_SPOT;
			light->position = parse_vec3(xmlGetProp(cur_node, "position"));
			light->direction = parse_vec3(xmlGetProp(cur_node, "direction"));
			light->angle = parse_double(xmlGetProp(cur_node, "angle"));
		} else 
		{
			printf("Unknown light type: \"%s\"\n", cur_node->name);
			return false;
		}

		light->colour = parse_colour(xmlGetProp(cur_node, "color"));
		light->intensity = parse_double(xmlGetProp(cur_node, "intensity"));
		light->name = strdup(xmlGetProp(cur_node, "name"));
	}
	assert(i == n);

	return true;
}

static bool import_shapes(Sdl *sdl, xmlNode *node, int n)
{
	return true;
}

static bool import_textures(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;

	sdl->num_textures = n;
	for (i = 0, cur_node = node; cur_node; i++, cur_node = xmlNextElementSibling(cur_node))
	{
	}

}

static bool import_materials(Sdl *sdl, xmlNode *node, int n)
{
	return true;
}

static bool import_scene(Sdl *sdl, xmlNode *node, int n)
{
	return true;
}

static bool import_sdl(Sdl *sdl, xmlDoc *doc)
{
	xmlNode *root, *node;

	root = xmlDocGetRootElement(doc);
	if (root == NULL)
	{
		printf("No root element\n");
		return false;
	}

	for (node = xmlFirstElementChild(root); node != NULL;
			node = xmlNextElementSibling(node))
	{
		int n;
		assert(node->type == XML_ELEMENT_NODE);

		n = xmlChildElementCount(node);
		printf("%s: %d nodes\n", node->name, n);

		if (strcmp(node->name, "Cameras") == 0)
		{
			if (!import_cameras(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Lights") == 0)
		{
			if (!import_lights(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Geometry") == 0)
		{
			if (!import_shapes(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Textures") == 0)
		{
			if (!import_textures(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Materials") == 0)
		{
			if (!import_materials(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Scene") == 0)
		{
			if (!import_scene(sdl, node, n))
				return false;
		} else
			printf("Unknown node: %s\n", node->name);
	}

	return true;
}

Sdl *sdl_load(const char *filename)
{
	Sdl *sdl = NULL;
	xmlDoc *doc = NULL;

	sdl = malloc(sizeof(Sdl));

	LIBXML_TEST_VERSION

	doc = xmlReadFile(filename, NULL, XML_PARSE_DTDLOAD | XML_PARSE_DTDVALID);
	if (doc == NULL)
	{
		printf("Error opening file %s\n", filename);
		goto errorout;
	}

	if (!import_sdl(sdl, doc))
	{
		printf("Error importing SDL from file %s\n", filename);
		goto errorout;
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();
	return sdl;
errorout:
	free(sdl);
	if (doc) xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
}
