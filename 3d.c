#include <math.h>
#include <stdlib.h>
#include "3d.h"
#include "internal.h"
#include "math.h"

#define MAX_ATTRIBUTES 12

static ShaderAttributes attr0;
static ShaderAttributes attr1;
static ShaderAttributes attr2;
static ShaderAttributes attr_frag;

void init_3d() {
	attr0.attributes = malloc(sizeof(ShaderAttribute) * MAX_ATTRIBUTES);
	attr1.attributes = malloc(sizeof(ShaderAttribute) * MAX_ATTRIBUTES);
	attr2.attributes = malloc(sizeof(ShaderAttribute) * MAX_ATTRIBUTES);
	attr_frag.attributes = malloc(sizeof(ShaderAttribute) * MAX_ATTRIBUTES);
}

double edge_function(const Vec3f *a, const Vec3f *b, const Vec3f *p) {
	return (p->x - a->x) * (b->y - a->y) - (p->y - a->y) * (b->x - a->x);
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

void print_polygon(Polygon *polygon, void *data, VertexShader vertex_shader, FragmentShader fragment_shader) {
	int *order = polygon->faces;
	VertexShaderOutput v0, v1, v2;
	Vec3f screen_pos0, screen_pos1, screen_pos2;
	Vec3f pos;

	for (int i = 0; i < polygon->n_faces; i++, order += 3) {
		attr0.n = 0, attr1.n = 0, attr2.n = 0;
		v0 = vertex_shader(polygon->points + order[0], data, &attr0);
		v1 = vertex_shader(polygon->points + order[1], data, &attr1);
		v2 = vertex_shader(polygon->points + order[2], data, &attr2);
		screen_pos0 = clip_to_screen(v0.clip_pos);
		screen_pos1 = clip_to_screen(v1.clip_pos);
		screen_pos2 = clip_to_screen(v2.clip_pos);

		double area = edge_function(&screen_pos0, &screen_pos1, &screen_pos2);
		if (area < 1e-6) {
			continue;
		}

		float one_over_w0 = (fabs(v0.clip_pos.w) < 1e-6) ? 0 : 1.f / v0.clip_pos.w;
		float one_over_w1 = (fabs(v1.clip_pos.w) < 1e-6) ? 0 : 1.f / v1.clip_pos.w;
		float one_over_w2 = (fabs(v2.clip_pos.w) < 1e-6) ? 0 : 1.f / v2.clip_pos.w;

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
						double interpolated_one_over_w_raw = one_over_w0 * w0 + one_over_w1 * w1 + one_over_w2 * w2;
						if (fabs(interpolated_one_over_w_raw) < 1e-10) {
							continue;
						}
						double one_over_interpolated_one_over_w_raw = 1.0 / interpolated_one_over_w_raw;

						attr_frag.n = attr0.n;
						for (int i = 0; i < attr_frag.n; i++) {
							Vec4f v0ow = vec4f_scale(attr0.attributes[i].value.vec4f, one_over_w0);
							Vec4f v1ow = vec4f_scale(attr1.attributes[i].value.vec4f, one_over_w1);
							Vec4f v2ow = vec4f_scale(attr2.attributes[i].value.vec4f, one_over_w2);
							Vec4f interpolated_v_over_w_raw = vec4f_add(
								vec4f_scale(v0ow, w0),
								vec4f_add(
									vec4f_scale(v1ow, w1),
									vec4f_scale(v2ow, w2)
								)
							);
							attr_frag.attributes[i].type = attr0.attributes[i].type;
							attr_frag.attributes[i].value.vec4f = vec4f_scale(interpolated_v_over_w_raw, one_over_interpolated_one_over_w_raw);
						}

						Pixel fragment = fragment_shader(&attr_frag, data);

						frame_buffer[y * view_width + x] = fragment;
						z_buffer[y * view_width + x] = pos.z;
					}
				}
			}
		}
	}
}

