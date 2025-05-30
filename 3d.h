#ifndef LIB3C_3D_H
#define LIB3C_3D_H

#include "model.h"

typedef enum {
	VEC4F
} ShaderAttributeType;

typedef struct {
	ShaderAttributeType type;
	union {
		Vec4f vec4f;
	} value;
} ShaderAttribute;

typedef struct {
	int n;
	ShaderAttribute *attributes;
} ShaderAttributes;

typedef struct {
	Vec4f screen_pos;
} FragmentShaderInput;

typedef struct {
	Vec4f clip_pos;
} VertexShaderOutput;

typedef Pixel (*FragmentShader)(FragmentShaderInput *input, ShaderAttributes *in, void *data);
typedef VertexShaderOutput (*VertexShader)(Point3D *point, void *data, ShaderAttributes *out);

void init_3d(void);
void print_polygon(Polygon *polygon, void *data, VertexShader vertex_shader, FragmentShader fragment_shader);

#endif
