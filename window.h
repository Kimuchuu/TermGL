#ifndef LIB3C_WINDOW_H
#define LIB3C_WINDOW_H

#include "math.h"

typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} Color;

typedef struct {
	int c;
	Color color;
} Pixel;

typedef struct {
	Vec3f position;
	Color color;
	Vec3f normal;
} Point3D;

typedef struct {
	Vec3f position;
	Vec3f target;
	Vec3f up;
	float fov_y;
} Camera;

typedef struct {
	Vec3f position;
	Color color;
	float ambient_strength;
	float specular_strength;
} Light;

typedef struct {
	Point3D *points;
	int n_points;
	int n_faces;
	int *faces;
} Polygon;

void cleanup();
void init(int width, int height, Camera *camera, Light *light);
void loop(int fps, void (*fn) (unsigned, double, double));
void print_triangle(Point3D *p0, Point3D *p1, Point3D *p2, Matrix4x4f *m_model, Matrix4x4f *m_view, Matrix4x4f *m_projection);
void print_polygon(Polygon *polygon, Matrix4x4f *m_model, Matrix4x4f *m_view, Matrix4x4f *m_projection);

#endif

