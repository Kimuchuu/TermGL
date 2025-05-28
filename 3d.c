#include <math.h>
#include <stdlib.h>
#include "3d.h"
#include "internal.h"

typedef struct {
	Vec4f clip_pos;
	Vec4f world_pos;
	Vec4f world_norm;

	float one_over_w;
	Vec3f world_pos_over_w;
	Vec3f world_norm_over_w;
	Vec3f ucolor_over_w;
} VertexShaderOutput;

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

double edge_function(const Vec3f *a, const Vec3f *b, const Vec3f *p) {
	return (p->x - a->x) * (b->y - a->y) - (p->y - a->y) * (b->x - a->x);
}

VertexShaderOutput vertex_shader(Point3D *point, Matrix4x4f *m_model, Matrix4x4f *m_model_it, Matrix4x4f *m_view, Matrix4x4f *m_projection) {
	Vec4f world_pos = { point->position.x, point->position.y, point->position.z, 1.f };
	world_pos = mat4f_multv(m_model, &world_pos);

	Vec4f point_view = mat4f_multv(m_view, &world_pos);

	Vec4f clip_pos = mat4f_multv(m_projection, &point_view);

	Vec4f world_norm = { point->normal.x, point->normal.y, point->normal.z, 0.f };
	world_norm = mat4f_multv(m_model_it, &world_norm);

	float one_over_w;
	Vec3f world_pos_over_w;
	Vec3f world_norm_over_w;
	Vec3f ucolor_over_w;
	if (fabs(clip_pos.w) < 1e-6) {
		one_over_w = 0;
		world_pos_over_w = (Vec3f) { 0, 0, 0 };
		world_norm_over_w = (Vec3f) { 0, 0, 0 };
		ucolor_over_w = (Vec3f) { 0, 0, 0 };
	} else {
		one_over_w = 1.f / clip_pos.w;
		world_pos_over_w = vec3f_scale(vec4f_to_vec3f(&world_pos), one_over_w);
		world_norm_over_w = vec3f_scale(vec4f_to_vec3f(&world_norm), one_over_w);
		ucolor_over_w = vec3f_scale(point->color, one_over_w);
	}

	VertexShaderOutput attrs = {
		.world_pos = world_pos,
		.clip_pos = clip_pos,
		.world_norm = world_norm,

		.one_over_w = one_over_w,
		.world_norm_over_w = world_norm_over_w,
		.world_pos_over_w = world_pos_over_w,
		.ucolor_over_w = ucolor_over_w,
	};

	return attrs;
}

Vec3f clip_to_screen(Vec4f clip) {
	Vec3f ndc, screen;
	ndc.x = clip.x / clip.w;
	ndc.y = clip.y / clip.w;
	ndc.z = clip.z / clip.w;
	screen.x = (ndc.x + 1.f) / 2.f * view_width;
	screen.y = (1.f - ndc.y) / 2.f * view_height;
	screen.z = (clip.z + 1.f) / 2.f;
	return screen;
}

void print_polygon(Polygon *polygon, Matrix4x4f *m_model, Matrix4x4f *m_view, Matrix4x4f *m_projection, FragmentShader fragment_shader) {
	Matrix4x4f m_model_it = mat4f_inverse_transpose_affine(m_model);
	int *order = polygon->faces;
	VertexShaderOutput v0, v1, v2;
	Vec3f screen_pos0, screen_pos1, screen_pos2;
	Vec3f pos;

	for (int i = 0; i < polygon->n_faces; i++, order += 3) {
		v0 = vertex_shader(polygon->points + order[0], m_model, &m_model_it, m_view, m_projection);
		v1 = vertex_shader(polygon->points + order[1], m_model, &m_model_it, m_view, m_projection);
		v2 = vertex_shader(polygon->points + order[2], m_model, &m_model_it, m_view, m_projection);
		screen_pos0 = clip_to_screen(v0.clip_pos);
		screen_pos1 = clip_to_screen(v1.clip_pos);
		screen_pos2 = clip_to_screen(v2.clip_pos);

		double area = edge_function(&screen_pos0, &screen_pos1, &screen_pos2);
		if (area < 1e-6) {
			continue;
		}

		double p_min_x = MIN(MIN(screen_pos0.x, screen_pos1.x), screen_pos2.x);
		double p_max_x = MAX(MAX(screen_pos0.x, screen_pos1.x), screen_pos2.x);
		double p_min_y = MIN(MIN(screen_pos0.y, screen_pos1.y), screen_pos2.y);
		double p_max_y = MAX(MAX(screen_pos0.y, screen_pos1.y), screen_pos2.y);

		unsigned int min_x = MAX(0, MIN(view_width - 1, floor(p_min_x)));
		unsigned int max_x = MAX(0, MIN(view_width - 1, floor(p_max_x)));
		unsigned int min_y = MAX(0, MIN(view_height - 1, floor(p_min_y)));
		unsigned int max_y = MAX(0, MIN(view_height - 1, floor(p_max_y)));
		for (unsigned int y = min_y; y <= max_y; y++) {
			for (unsigned int x = min_x; x <= max_x; x++) {
				pos.x = x;
				pos.y = y;
				double w0 = edge_function(&screen_pos1, &screen_pos2, &pos);
				double w1 = edge_function(&screen_pos2, &screen_pos0, &pos);
				double w2 = edge_function(&screen_pos0, &screen_pos1, &pos);
				if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
					w0 /= area;
					w1 /= area;
					w2 /= area;
					pos.z = screen_pos0.z * w0 + screen_pos1.z * w1 + screen_pos2.z * w2;
					if (pos.z < z_buffer[y * view_width + x]) {
						Vec3f interpolated_world_pos_over_w_raw = vec3f_add(
							vec3f_scale(v0.world_pos_over_w, w0),
							vec3f_add(
								vec3f_scale(v1.world_pos_over_w, w1),
								vec3f_scale(v2.world_pos_over_w, w2)
							)
						);

						Vec3f interpolated_world_normal_over_w_raw = vec3f_add(
							vec3f_scale(v0.world_norm_over_w, w0),
							vec3f_add(
								vec3f_scale(v1.world_norm_over_w, w1),
								vec3f_scale(v2.world_norm_over_w, w2)
							)
						);

						Vec3f interpolated_ucolor_over_w_raw = vec3f_add(
							vec3f_scale(v0.ucolor_over_w, w0),
							vec3f_add(
								vec3f_scale(v1.ucolor_over_w, w1),
								vec3f_scale(v2.ucolor_over_w, w2)
							)
						);

						double interpolated_one_over_w_raw = v0.one_over_w * w0 + v1.one_over_w * w1 + v2.one_over_w * w2;
						if (fabs(interpolated_one_over_w_raw) < 1e-10) {
							continue;
						}
						double one_over_interpolated_one_over_w_raw = 1.0 / interpolated_one_over_w_raw;
						Vec3f fragment_world_pos = vec3f_scale(interpolated_world_pos_over_w_raw, one_over_interpolated_one_over_w_raw);
						Vec3f fragment_world_normal = vec3f_scale(interpolated_world_normal_over_w_raw, one_over_interpolated_one_over_w_raw);
						Vec3f fragment_color_base = vec3f_scale(interpolated_ucolor_over_w_raw, one_over_interpolated_one_over_w_raw);
						Pixel fragment = fragment_shader(fragment_color_base, fragment_world_pos, fragment_world_normal, camera, lights, n_lights);

						frame_buffer[y * view_width + x] = fragment;
						z_buffer[y * view_width + x] = pos.z;
					}
				}
			}
		}
	}
}

