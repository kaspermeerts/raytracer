#ifndef CG_PPM
#define CG_PPM

#include <stdio.h>
#include <stdbool.h>
#include "colour.h"

bool ppm_write(Colour *buffer, int width, int height, FILE *fd);

#endif
