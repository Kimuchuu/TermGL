#include "simple3d.h"
#include "3d.h"
#include "math.h"
#include <math.h>
#include <stdlib.h>

static char depth[] = { '@', '$', '#', '*', '!', '=', ';', ':', '~', '-', ',', '.' };
static Camera *camera;
static Light **lights;
static int size_lights;
static int n_lights;

void init_camera(Camera *i_camera) {
	camera = i_camera;
}

void add_light(Light *light) {
	if (size_lights == 0) {
		lights = calloc(10, sizeof(Light *));
		size_lights = 10;
	} else if (n_lights == size_lights) {
		lights = realloc(lights, (size_lights + 10) * sizeof(Light *));
		size_lights += 10;
		for (int i = size_lights - 10; i < size_lights; i++) {
			lights[i] = NULL;
		}
	}
	for (int i = 0; i < size_lights; i++) {
		if (lights[i] == NULL) {
			lights[i] = light;
			n_lights++;
			break;
		}
	}
}

void remove_light(Light *light) {
	for (int i = 0; i < size_lights; i++) {
		if (lights[i] == light) {
			lights[i] = NULL;
			if (i < size_lights - 1) {
				for (int j = i + 1; j < size_lights; j++) {
					lights[j-1] = lights[j];
				}
				lights[size_lights - 1] = NULL;
			}
			n_lights--;
			break;
		}
	}

	if (n_lights < (size_lights - 40) && size_lights > 40) {
		lights = realloc(lights, size_lights - 20);
		size_lights -= 20;
	}
}

Pixel fragment_shader_ascii(FragmentShaderInput *input, ShaderAttributes *in, void *data) {
	Vec3f world_pos = vec4f_to_vec3f(&in->attributes[1].value.vec4f);
	Vec3f world_norm = vec4f_to_vec3f(&in->attributes[2].value.vec4f);
	Vec3f world_norm_normalized = vec3f_normalize(world_norm);
	float total_diffuse_strength = 0;
	for (int i = 0; i < n_lights; i++) {
		Vec3f light_direction = vec3f_normalize(vec3f_sub(lights[i]->position, world_pos));
		float diffuse_strength = MAX(vec3f_dot(world_norm_normalized, light_direction), 0.f);
		total_diffuse_strength += diffuse_strength;
	}

	Pixel fragment;
	short d_index = (1 - total_diffuse_strength) * (sizeof(depth) - 1);
	fragment.c = depth[d_index];
	fragment.color = (Color) { 1, 1, 1 };
	return fragment;
}

Pixel fragment_shader_ascii_color(FragmentShaderInput *input, ShaderAttributes *in, void *data) {
	Color color = vec4f_to_vec3f(&in->attributes[0].value.vec4f);
	Vec3f world_pos = vec4f_to_vec3f(&in->attributes[1].value.vec4f);
	Vec3f world_norm = vec4f_to_vec3f(&in->attributes[2].value.vec4f);
	Vec3f view_direction = vec3f_normalize(vec3f_sub(camera->position, world_pos));
	Vec3f world_norm_normalized = vec3f_normalize(world_norm);

	float total_diffuse_strength = 0;
	Vec3f final_color;
	final_color.x = 0, final_color.y = 0, final_color.z = 0;
	for (int i = 0; i < n_lights; i++) {
		Vec3f light_direction = vec3f_normalize(vec3f_sub(lights[i]->position, world_pos));
		Vec3f reflect_direction = vec3f_reflect(vec3f_scale(light_direction, -1.f), world_norm_normalized);
		Vec3f ambient_ucolor = vec3f_scale(lights[i]->color, lights[i]->ambient_strength);
		float diffuse_strength = MAX(vec3f_dot(world_norm_normalized, light_direction), 0.f);
		total_diffuse_strength += diffuse_strength;
		Vec3f diffuse_ucolor = vec3f_scale(lights[i]->color, diffuse_strength);
		float spec = powf(MAX(vec3f_dot(view_direction, reflect_direction), 0.f), 32.f);
		Vec3f specular_ucolor = vec3f_scale(lights[i]->color, lights[i]->specular_strength * spec);

		Vec3f light_color = vec3f_mult(
			vec3f_add(vec3f_add(ambient_ucolor, diffuse_ucolor), specular_ucolor),
			color);

		final_color.x += light_color.x;
		final_color.y += light_color.y;
		final_color.z += light_color.z;
	}
	final_color.x = MAX(0, MIN(1, (final_color.x)));
	final_color.y = MAX(0, MIN(1, (final_color.y)));
	final_color.z = MAX(0, MIN(1, (final_color.z)));

	Pixel fragment;
	short d_index = (1 - total_diffuse_strength) * (sizeof(depth) - 1);
	fragment.c = depth[d_index];
	fragment.color = final_color;
	return fragment;
}

Pixel fragment_shader_color(FragmentShaderInput *input, ShaderAttributes *in, void *data) {
	Color color = vec4f_to_vec3f(&in->attributes[0].value.vec4f);
	Vec3f world_pos = vec4f_to_vec3f(&in->attributes[1].value.vec4f);
	Vec3f world_norm = vec4f_to_vec3f(&in->attributes[2].value.vec4f);
	Vec3f view_direction = vec3f_normalize(vec3f_sub(camera->position, world_pos));
	Vec3f world_norm_normalized = vec3f_normalize(world_norm);

	Vec3f final_color;
	final_color.x = 0, final_color.y = 0, final_color.z = 0;
	for (int i = 0; i < n_lights; i++) {
		Vec3f light_direction = vec3f_normalize(vec3f_sub(lights[i]->position, world_pos));
		Vec3f reflect_direction = vec3f_reflect(vec3f_scale(light_direction, -1.f), world_norm_normalized);
		Vec3f ambient_ucolor = vec3f_scale(lights[i]->color, lights[i]->ambient_strength);
		float diffuse_strength = MAX(vec3f_dot(world_norm_normalized, light_direction), 0.f);
		Vec3f diffuse_ucolor = vec3f_scale(lights[i]->color, diffuse_strength);
		float spec = powf(MAX(vec3f_dot(view_direction, reflect_direction), 0.f), 32.f);
		Vec3f specular_ucolor = vec3f_scale(lights[i]->color, lights[i]->specular_strength * spec);

		Vec3f light_color = vec3f_mult(
			vec3f_add(vec3f_add(ambient_ucolor, diffuse_ucolor), specular_ucolor),
			color);

		final_color.x += light_color.x;
		final_color.y += light_color.y;
		final_color.z += light_color.z;
	}
	final_color.x = MAX(0, MIN(1, (final_color.x)));
	final_color.y = MAX(0, MIN(1, (final_color.y)));
	final_color.z = MAX(0, MIN(1, (final_color.z)));

	Pixel fragment;
	fragment.c = depth[0];
	fragment.color = final_color;
	return fragment;
}

Pixel fragment_shader_debug(FragmentShaderInput *input, ShaderAttributes *in, void *data) {
	Vec3f world_norm = vec4f_to_vec3f(&in->attributes[2].value.vec4f);
	Vec3f world_norm_normalized = vec3f_normalize(world_norm);
	Pixel fragment;
	fragment.c = depth[0];
	fragment.color.x   = (world_norm_normalized.x + 1.f) / 2.f;
	fragment.color.y = (world_norm_normalized.y + 1.f) / 2.f;
	fragment.color.z  = (world_norm_normalized.z + 1.f) / 2.f;
	return fragment;
}

VertexShaderOutput vertex_shader_simple(Point3D *point, void *data, ShaderAttributes *out) {
	SimpleShaderData *uniform = data;
	Vec4f world_pos = { point->position.x, point->position.y, point->position.z, 1.f };
	world_pos = mat4f_multv(uniform->m_model, &world_pos);

	Vec4f point_view = mat4f_multv(uniform->m_view, &world_pos);
	Vec4f clip_pos = mat4f_multv(uniform->m_projection, &point_view);
	Vec4f world_norm = { point->normal.x, point->normal.y, point->normal.z, 0.f };
	world_norm = mat4f_multv(uniform->m_model_it, &world_norm);

	out->n = 3;
	out->attributes[0].type = VEC4F;
	out->attributes[0].value.vec4f = vec3f_to_vec4f(&point->color, 1.f);
	out->attributes[1].type = VEC4F;
	out->attributes[1].value.vec4f = world_pos;
	out->attributes[2].type = VEC4F;
	out->attributes[2].value.vec4f = world_norm;

	VertexShaderOutput output = { .clip_pos = clip_pos };
	return output;
}

