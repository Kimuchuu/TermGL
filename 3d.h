#ifndef LIB3C_3D_H
#define LIB3C_3D_H

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

typedef Pixel (*FragmentShader)(Vec3f color, Vec3f world_pos, Vec3f world_norm, Camera *camera, Light **lights, int n_lights);

void init_camera(Camera *i_camera);

void add_light(Light *light);
void remove_light(Light *light);
void print_polygon(Polygon *polygon, Matrix4x4f *m_model, Matrix4x4f *m_view, Matrix4x4f *m_projection, FragmentShader fragment_shader);

#endif
