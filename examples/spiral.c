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
	float time;
} ShaderData;

static Polygon polygon;
static ShaderData shader_data;

VertexShaderOutput vertex_shader(Point3D *point, void *data, ShaderAttributes *out) {
	Vec4f world_pos = { point->position.x, point->position.y, point->position.z, 1.f };
	VertexShaderOutput output = { .clip_pos = world_pos };
	return output;
}

Pixel fragment_spiral(FragmentShaderInput *input, ShaderAttributes *in, void *data) {
	ShaderData *uniform = data;

	float radius = uniform->resolution.x / 2.f;
	float center_x = uniform->resolution.x / 2.f;
	float center_y = uniform->resolution.y / 2.f;

	float translated_x = (input->screen_pos.x - center_x) * (1.0f / uniform->aspect_ratio);
	float translated_y = input->screen_pos.y - center_y;

	float angle = atan2f(translated_y, translated_x);
	float dist = sqrtf(translated_x * translated_x + translated_y * translated_y);
	float spiral_val = angle + dist * 21;
	float animated_spiral_val = spiral_val + uniform->time * 0.005;

	Pixel fragment;
	fragment.c = '#';
	fragment.color.x = (sinf(animated_spiral_val) + 1.f) * .5f;
	fragment.color.y = (sinf(animated_spiral_val + (M_PI * 2.0f / 3.0f)) + 1.f) * .5f;
	fragment.color.z = (sinf(animated_spiral_val + (M_PI * 4.0f / 3.0f)) + 1.f) * .5f;
	return fragment;
}

void draw(unsigned int frame, double time, double delta) {
	static float degrees = 0.f;
	degrees += (delta / 1000.f) * 90.f;
	shader_data.time = time;
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
