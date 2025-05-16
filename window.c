#include "window.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "math.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

typedef struct {
	int c;
	Color color;
} Pixel;

static struct timespec timer;

typedef struct {
	Vec3f screen_pos;
	Vec4f clip_pos;
	Vec4f world_pos;
} PointPosAttrs;

typedef struct {
	Vec4f world_norm;
	float one_over_w;
	Vec3f world_pos_over_w;
	Vec3f world_norm_over_w;
	Vec3f ucolor_over_w;
} PointExtraAttrs;

static int view_height;
static int view_width;

static Pixel *frame_buffer;
static double *z_buffer;
static char *print_buffer;

static char depth[] = { '@', '$', '#', '*', '!', '=', ';', ':', '~', '-', ',', '.' };

static Camera *camera;
static Light *light;

static void clear_buffers() {
	for (int i = 0; i < view_height; i++) {
		for (int j = 0; j < view_width; j++) {
			frame_buffer[i * view_width + j].c = ' ';
			z_buffer[i * view_width + j] = DBL_MAX;
		}
	}
}

static void flush_buffers() {
	printf("\x1b[H\x1b[0J");
	for (int i = 0; i < view_height; i++) {
		printf("\x1b[%d;1H", i + 1);
		for (int j = 0; j < view_width; j++) {
			printf(
				"\x1b[38;2;%d;%d;%dm%c",
				(unsigned char)(frame_buffer[i * view_width + j].color.x * 255.f),
				(unsigned char)(frame_buffer[i * view_width + j].color.y * 255.f),
				(unsigned char)(frame_buffer[i * view_width + j].color.z * 255.f),
				frame_buffer[i * view_width + j].c
			);
		}
	}
	fflush(stdout);
}

double get_time_ms() {
	clock_gettime(CLOCK_MONOTONIC, &timer);
	return ((double)timer.tv_nsec / 1000000.0) + ((double)timer.tv_sec * 1000.0);
}

void init(int width, int height, Camera *i_camera, Light *i_light) {
	view_width = width;
	view_height = height;
	camera = i_camera;
	light = i_light;

	frame_buffer = malloc(height * width * sizeof(frame_buffer[0]));
	z_buffer = malloc(height * width * sizeof(z_buffer[0]));
	int print_buffer_size = height * width * 32 * sizeof(print_buffer[0]);
	print_buffer = malloc(print_buffer_size);
	// Prevents writing too early (stdout is line buffered), less flickering
	setvbuf(stdout, print_buffer, _IOFBF, print_buffer_size);
	printf("\x1b[?25l"); // Hide cursor
}

void cleanup() {
	printf("\x1b[?25h"); // Show cursor
	fflush(stdout);
	free(print_buffer);
	free(z_buffer);
	free(frame_buffer);
}

void loop(int fps, void (*fn) (unsigned, double, double)) {
	unsigned int frame = 0;
	double interval_ms = 1000.0 / fps;
	double time_last = get_time_ms();
	double time_frame, time, delta;

	while (1) {
		time = get_time_ms();
		delta = time - time_last;
		// START DRAW
		clear_buffers();
		fn(frame, time, delta);
		flush_buffers();
		// END DRAW
		time_frame = get_time_ms() - time;
		if (time_frame < interval_ms) {
			usleep((interval_ms - time_frame) * 1000);
		}
		time_last = time;
		frame++;
	}
}

PointPosAttrs create_pos_attrs(Point3D *point, Matrix4x4f *m_model, Matrix4x4f *m_view, Matrix4x4f *m_projection) {
	Vec4f point_world = { point->position.x, point->position.y, point->position.z, 1.f };
	point_world = mat4f_multv(m_model, &point_world);

	Vec4f point_view = mat4f_multv(m_view, &point_world);

	Vec4f point_clip = mat4f_multv(m_projection, &point_view);

	Vec2f point_ndc;
	point_ndc.x = point_clip.x / point_clip.w;
	point_ndc.y = point_clip.y / point_clip.w;

	Vec3f point_screen;
	point_screen.x = (point_ndc.x + 1.f) / 2.f * view_width;
	point_screen.y = (1.f - point_ndc.y) / 2.f * view_height;
	point_screen.z = -point_view.z;

	PointPosAttrs attrs = {
		.world_pos = point_world,
		.clip_pos = point_clip,
		.screen_pos = point_screen,
	};

	return attrs;
}

PointExtraAttrs create_extra_attrs(PointPosAttrs *pos_attrs, Point3D *point, Matrix4x4f *m_model_it) {
	Vec4f normal_world = { point->normal.x, point->normal.y, point->normal.z, 0.f };
	normal_world = mat4f_multv(m_model_it, &normal_world);

	float one_over_w;
	Vec3f world_pos_over_w;
	Vec3f world_norm_over_w;
	Vec3f ucolor_over_w;
	if (fabs(pos_attrs->clip_pos.w) < 1e-6) {
		one_over_w = 0;
		world_pos_over_w = (Vec3f) { 0, 0, 0 };
		world_norm_over_w = (Vec3f) { 0, 0, 0 };
		ucolor_over_w = (Vec3f) { 0, 0, 0 };
	} else {
		one_over_w = 1.f / pos_attrs->clip_pos.w;
		world_pos_over_w = vec3f_scale(vec4f_to_vec3f(&pos_attrs->world_pos), one_over_w);
		world_norm_over_w = vec3f_scale(vec4f_to_vec3f(&normal_world), one_over_w);
		ucolor_over_w = vec3f_scale(point->color, one_over_w);
	}

	PointExtraAttrs attrs = {
		.world_norm = normal_world,
		.one_over_w = one_over_w,
		.world_norm_over_w = world_norm_over_w,
		.world_pos_over_w = world_pos_over_w,
		.ucolor_over_w = ucolor_over_w,
	};

	return attrs;
}

double edge_function(const Vec3f *a, const Vec3f *b, const Vec3f *p) {
	return (p->x - a->x) * (b->y - a->y) - (p->y - a->y) * (b->x - a->x);
}

Vec3f create_normal(Vec4f *p0, Vec4f *p1, Vec4f *p2) {
	Vec3f v0 = vec4f_to_vec3f(p0);
	Vec3f v1 = vec4f_to_vec3f(p1);
	Vec3f v2 = vec4f_to_vec3f(p2);

	Vec3f A = vec3f_sub(v1, v0);
	Vec3f B = vec3f_sub(v2, v0);
	Vec3f normal = vec3f_normalize((Vec3f) {
		A.y * B.z - A.z * B.y,
		A.z * B.x - A.x * B.z,
		A.x * B.y - A.y * B.x,
	});

	return normal;
}

void print_polygon(Polygon *polygon, Matrix4x4f *m_model, Matrix4x4f *m_view, Matrix4x4f *m_projection) {
	Matrix4x4f m_model_it = mat4f_inverse_transpose_affine(m_model);
	int *order = polygon->faces;
	PointExtraAttrs ea0, ea1, ea2;
	PointPosAttrs pa0, pa1, pa2;
	Point3D *p0, *p1, *p2;
	Vec3f pos;
	Color color;
	for (int i = 0; i < polygon->n_faces; i++, order += 3) {
		p0 = polygon->points + order[0];
		p1 = polygon->points + order[1];
		p2 = polygon->points + order[2];
		pa0 = create_pos_attrs(p0, m_model, m_view, m_projection);
		pa1 = create_pos_attrs(p1, m_model, m_view, m_projection);
		pa2 = create_pos_attrs(p2, m_model, m_view, m_projection);

		double area = edge_function(&pa0.screen_pos, &pa1.screen_pos, &pa2.screen_pos);
		if (area < 1e-6) {
			continue;
		}

		double p_min_x = MIN(MIN(pa0.screen_pos.x, pa1.screen_pos.x), pa2.screen_pos.x);
		double p_max_x = MAX(MAX(pa0.screen_pos.x, pa1.screen_pos.x), pa2.screen_pos.x);
		double p_min_y = MIN(MIN(pa0.screen_pos.y, pa1.screen_pos.y), pa2.screen_pos.y);
		double p_max_y = MAX(MAX(pa0.screen_pos.y, pa1.screen_pos.y), pa2.screen_pos.y);

		unsigned int min_x = MAX(0, MIN(view_width - 1, floor(p_min_x)));
		unsigned int max_x = MAX(0, MIN(view_width - 1, floor(p_max_x)));
		unsigned int min_y = MAX(0, MIN(view_height - 1, floor(p_min_y)));
		unsigned int max_y = MAX(0, MIN(view_height - 1, floor(p_max_y)));
		char processed_extra_attrs = 0;
		for (unsigned int y = min_y; y <= max_y; y++) {
			for (unsigned int x = min_x; x <= max_x; x++) {
				pos.x = x;
				pos.y = y;
				double w0 = edge_function(&pa1.screen_pos, &pa2.screen_pos, &pos);
				double w1 = edge_function(&pa2.screen_pos, &pa0.screen_pos, &pos);
				double w2 = edge_function(&pa0.screen_pos, &pa1.screen_pos, &pos);
				if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
					w0 /= area;
					w1 /= area;
					w2 /= area;
					pos.z = pa0.screen_pos.z * w0 + pa1.screen_pos.z * w1 + pa2.screen_pos.z * w2;
					if (pos.z < z_buffer[y * view_width + x]) {
						if (!processed_extra_attrs) {
							processed_extra_attrs = 1;
							ea0 = create_extra_attrs(&pa0, p0, &m_model_it);
							ea1 = create_extra_attrs(&pa1, p1, &m_model_it);
							ea2 = create_extra_attrs(&pa2, p2, &m_model_it);
						}
						Vec3f interpolated_world_pos_over_w_raw = vec3f_add(
							vec3f_scale(ea0.world_pos_over_w, w0),
							vec3f_add(
								vec3f_scale(ea1.world_pos_over_w, w1),
								vec3f_scale(ea2.world_pos_over_w, w2)
							)
						);

						Vec3f interpolated_world_normal_over_w_raw = vec3f_add(
							vec3f_scale(ea0.world_norm_over_w, w0),
							vec3f_add(
								vec3f_scale(ea1.world_norm_over_w, w1),
								vec3f_scale(ea2.world_norm_over_w, w2)
							)
						);

						Vec3f interpolated_ucolor_over_w_raw = vec3f_add(
							vec3f_scale(ea0.ucolor_over_w, w0),
							vec3f_add(
								vec3f_scale(ea1.ucolor_over_w, w1),
								vec3f_scale(ea2.ucolor_over_w, w2)
							)
						);


						double interpolated_one_over_w_raw = ea0.one_over_w * w0 + ea1.one_over_w * w1 + ea2.one_over_w * w2;
						if (fabs(interpolated_one_over_w_raw) < 1e-10) {
							continue;
						}
						double one_over_interpolated_one_over_w_raw = 1.0 / interpolated_one_over_w_raw;
						Vec3f fragment_world_pos = vec3f_scale(interpolated_world_pos_over_w_raw, one_over_interpolated_one_over_w_raw);
						Vec3f fragment_world_normal = vec3f_scale(interpolated_world_normal_over_w_raw, one_over_interpolated_one_over_w_raw);
						Vec3f fragment_world_normal_normalized = vec3f_normalize(fragment_world_normal);
						Vec3f fragment_color_base = vec3f_scale(interpolated_ucolor_over_w_raw, one_over_interpolated_one_over_w_raw);

						Vec3f light_direction = vec3f_normalize(vec3f_sub(light->position, fragment_world_pos));
						Vec3f view_direction = vec3f_normalize(vec3f_sub(camera->position, fragment_world_pos));
						Vec3f reflect_direction = vec3f_reflect(vec3f_scale(light_direction, -1.f), fragment_world_normal_normalized);

						Vec3f ambient_ucolor = vec3f_scale(light->color, light->ambient_strength);
						float diffuse_strength = MAX(vec3f_dot(fragment_world_normal_normalized, light_direction), 0.f);
						Vec3f diffuse_ucolor = vec3f_scale(light->color, diffuse_strength);
						float spec = powf(MAX(vec3f_dot(view_direction, reflect_direction), 0.f), 32.f);
						Vec3f specular_ucolor = vec3f_scale(light->color, light->specular_strength * spec);

						Vec3f final_color = vec3f_mult(
							vec3f_add(vec3f_add(ambient_ucolor, diffuse_ucolor), specular_ucolor),
							fragment_color_base);

						color.x = MAX(0, MIN(1, (final_color.x)));
						color.y = MAX(0, MIN(1, (final_color.y)));
						color.z = MAX(0, MIN(1, (final_color.z)));


						// NORMAL DEBUGGING
						// frame_buffer[y * view_width + x].c = depth[0];
						// frame_buffer[y * view_width + x].color.red   = (fragment_world_normal_normalized.x + 1.f) / 2.f;
						// frame_buffer[y * view_width + x].color.green = (fragment_world_normal_normalized.y + 1.f) / 2.f;
						// frame_buffer[y * view_width + x].color.blue  = (fragment_world_normal_normalized.z + 1.f) / 2.f;

						// ASCII LIGHT
						// short d_index = (1 - diffuse_strength) * (sizeof(depth) - 1);
						// frame_buffer[y * view_width + x].c = depth[d_index];
						// frame_buffer[y * view_width + x].color = (Color) { 1, 1, 1 };

						// COLOR
						frame_buffer[y * view_width + x].c = depth[0];
						frame_buffer[y * view_width + x].color = color;

						// ASCII LIGHT AND COLOR
						// short d_index = (1 - diffuse_strength) * (sizeof(depth) - 1);
						// frame_buffer[y * view_width + x].c = depth[d_index];
						// frame_buffer[y * view_width + x].color = color;

						z_buffer[y * view_width + x] = pos.z;
					}
				}
			}
		}
	}
}
