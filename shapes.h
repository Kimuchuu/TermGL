#ifndef LIB3C_SHAPES_H
#define LIB3C_SHAPES_H

#include "model.h"
#include "math.h"

Polygon s_circle(float radius, int steps);
Polygon s_cube(float width, float height, float depth);
Polygon s_pyramid(float width, float height, float depth);
Polygon s_sphere(float radius, int stacks, int sectors);

#endif
