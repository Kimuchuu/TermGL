#ifndef LIB3C_MODEL_H
#define LIB3C_MODEL_H

#include "math.h"

typedef Vec3f Color;

typedef struct {
	int c;
	Color color;
} Pixel;

typedef struct {
	Vec3f position;
	Color color;
	Vec3f normal;
	Vec2f uv;
} Point3D;

typedef struct {
	Point3D *points;
	int n_points;
	int n_faces;
	int *faces;
} Polygon;

#endif

