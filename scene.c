#include "scene.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

Scene *scene_load(const char *filename)
{
	xmlDoc *doc;

	doc = xmlReadFile(filename, NULL, XML_PARSE_DTDVALID);
}
