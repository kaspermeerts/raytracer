#ifndef CG_PPM
#define CG_PPM

#include <stdio.h>
#include <stdbool.h>
#include "colour.h"

bool write_ppm_file(Colour *buffer, int width, int height, FILE *fd);

#endif
