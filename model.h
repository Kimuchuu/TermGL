#ifndef LIB3C_MODEL_H
#define LIB3C_MODEL_H

#include "math.h"

typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} Color;

typedef struct {
	Vec3f position;
	Color color;
	Vec3f normal;
} Point3D;

typedef struct {
	Point3D *points;
	int n_points;
	int n_faces;
	int *faces;
} Polygon;

#endif

