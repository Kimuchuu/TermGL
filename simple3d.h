#ifndef LIB3C_SIMPLE3D_H
#define LIB3C_SIMPLE3D_H

#include "3d.h"
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

typedef struct {
	Matrix4x4f *m_model;
	Matrix4x4f *m_model_it;
	Matrix4x4f *m_view;
	Matrix4x4f *m_projection;
} SimpleShaderData;


void init_camera(Camera *);

void add_light(Light *);
void remove_light(Light *);

Pixel fragment_shader_ascii(FragmentShaderInput *input, ShaderAttributes *in, void *data);
Pixel fragment_shader_ascii_color(FragmentShaderInput *input, ShaderAttributes *in, void *data);
Pixel fragment_shader_color(FragmentShaderInput *input, ShaderAttributes *in, void *data);
Pixel fragment_shader_debug(FragmentShaderInput *input, ShaderAttributes *in, void *data);

VertexShaderOutput vertex_shader_simple(Point3D *point, void *data, ShaderAttributes *out);

#endif
