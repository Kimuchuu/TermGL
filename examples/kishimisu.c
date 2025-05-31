//
// Shader taken from kishimisu: An introduction to Shader Art Coding
//

#include <bits/time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include "../window.h"
#include "../3d.h"

typedef struct {
	Vec2f resolution;
	float aspect_ratio;
	Vec2f pixel_resolution;
	float pixel_aspect_ratio;
	float time;
} ShaderData;

static Polygon polygon;
static ShaderData shader_data;

VertexShaderOutput vertex_shader(Point3D *point, void *data, ShaderAttributes *out) {
	Vec4f world_pos = { point->position.x, point->position.y, point->position.z, 1.f };
	VertexShaderOutput output = { .clip_pos = world_pos };
	return output;
}

Vec2f vec2f_div(Vec2f v1, float f) {
	v1.x /= f;
	v1.y /= f;
	return v1;
}
Vec2f vec2f_sub_s(Vec2f v1, float f) {
	v1.x -= f;
	v1.y -= f;
	return v1;
}
float vec2f_len(Vec2f v) {
	return sqrtf(v.x * v.x + v.y * v.y);
}
Vec2f fract(Vec2f v) {
	v.x -= floorf(v.x);
	v.y -= floorf(v.y);
	return v;
}


Vec3f palette(float t) {
	Vec3f a = { 0.5f, 0.5f, 0.5f };
	Vec3f b = { 0.5f, 0.5f, 0.5f };
	Vec3f c = { 1.0f, 1.0f, 1.0f };
	Vec3f d = { 0.263f, 0.816f, 0.57f };
	Vec3f output;
	output.x = a.x + b.x * cos(2.0 * M_PI * (c.x * t + d.x));
	output.y = a.y + b.y * cos(2.0 * M_PI * (c.y * t + d.y));
	output.z = a.z + b.z * cos(2.0 * M_PI * (c.z * t + d.z));
	return output;
}

Pixel fragment_spiral(FragmentShaderInput *input, ShaderAttributes *in, void *data) {
	ShaderData *uniform = data;
	Pixel fragment;
	// Fixes the inconsistent row/col heights
	Vec2f frag_coord = {
		input->screen_pos.x / uniform->resolution.x * uniform->pixel_resolution.x,
		input->screen_pos.y / uniform->resolution.y * uniform->pixel_resolution.y,
	};
	Vec2f uv = vec2f_div(vec2f_sub(vec2f_scale(frag_coord, 2), uniform->pixel_resolution), uniform->pixel_resolution.y);
	Vec2f uv0 = uv;
	Vec3f final_color = {};
	float d;
	for (float i = 0.f; i < 4.f; i++) {
		uv = vec2f_scale(uv, 1.5f);
		uv = fract(uv);
		uv = vec2f_sub_s(uv, 0.5f);

		d = vec2f_len(uv) * expf(-vec2f_len(uv0));

		Vec3f col = palette(vec2f_len(uv0) + i * .4f + uniform->time * .4f);

		d = sinf(d *8.f + uniform->time) / 8;
		d = fabs(d);

		d = powf(0.01f / d, 1.2f);

		final_color = vec3f_add(final_color, vec3f_scale(col, d));
	}

	fragment.c = '@' + (sinf(d) + 1) / 2 * 4;
	fragment.color.x = MIN(MAX(final_color.x, 0), 1);
	fragment.color.y = MIN(MAX(final_color.y, 0), 1);
	fragment.color.z = MIN(MAX(final_color.z, 0), 1);
	return fragment;
}

void draw(unsigned int frame, double time, double delta) {
	static float degrees = 0.f;
	degrees += (delta / 1000.f) * 90.f;
	shader_data.time = time / 1000;
	print_polygon(&polygon, &shader_data, vertex_shader, fragment_spiral);
}

void on_terminate(int code) {
	cleanup_window();
	exit(code);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, on_terminate);

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int width = w.ws_col;
	int height = w.ws_row;
	init_window(width, height);
	init_3d();

	shader_data.pixel_resolution.x = w.ws_xpixel;
	shader_data.pixel_resolution.y = w.ws_ypixel;
	shader_data.pixel_aspect_ratio = (float) w.ws_xpixel / w.ws_ypixel;

	shader_data.resolution.x = width;
	shader_data.resolution.y = height;
	shader_data.aspect_ratio = (float) width / height;

	polygon.n_points = 4;
	polygon.points = (Point3D[]) {
		{ .position =  { -1, -1, 0 } },
		{ .position =  { 1, -1, 0 } },
		{ .position =  { 1, 1, 0 } },
		{ .position =  { -1, 1, 0 } }
	};

	polygon.n_faces = 2;
	polygon.faces = (int[]) {
		0, 1, 2,
		0, 2, 3
	};

	loop(15, draw);

	return 0;
}
