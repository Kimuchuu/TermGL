#ifndef LIB3C_WINDOW_H
#define LIB3C_WINDOW_H

#include "model.h"

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

void cleanup();
void init(int width, int height, Camera *camera);
void loop(int fps, void (*fn) (unsigned, double, double));

void add_light(Light *light);
void remove_light(Light *light);

void print_polygon(Polygon *polygon, Matrix4x4f *m_model, Matrix4x4f *m_view, Matrix4x4f *m_projection);

#endif

