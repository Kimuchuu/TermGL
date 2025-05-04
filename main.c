#include <bits/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include "math.h"
#include "window.h"
#include "shapes.h"

typedef struct {
	Polygon polygon;
	Vec3f position;
} Object;

static double near = 0.1;
static double far = 100;
static Camera camera = {
	.position = { 0.0, 0.0, 3.0 },
	.target = { 0.0, 0.0, 0.0 },
	.up = { 0.0, 1.0, 0.0 },
	.fov_y = 45
};
static Light light = {
	.position = { 0.f, 5.f, 5.f },
	.color = { 255, 225, 255 },
	.ambient_strength = 0.1f,
	.specular_strength = 0.3f
};
static Matrix4x4f m_view;
static Matrix4x4f m_projection;
static Object objects[10];
static int n_objects;

void draw(unsigned int frame, double time, double delta) {
	static float degrees = 0.f;
	degrees += (delta / 1000.f) * 90.f;

	for (int i = 0; i < n_objects; i++) {
		Matrix4x4f translate = mat4f_translate(objects[i].position);
		Matrix4x4f rotate = mat4f_rotate_y(radians(degrees));
		Matrix4x4f model = mat4f_identity(1);
		model = mat4f_mult(&model, &translate);
		model = mat4f_mult(&model, &rotate);
		print_polygon(&objects[i].polygon, &model, &m_view, &m_projection);
	}
}

void on_terminate(int code) {
	cleanup();
	exit(code);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, on_terminate);

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int width = w.ws_col;
	int height = w.ws_row;
	init(width, height, &camera, &light);
	m_view = look_at(camera.position, camera.target, camera.up);
	m_projection = perspective(camera.fov_y,  width / (float) height, near, far);

	objects[n_objects++] = (Object) {
		s_pyramid(8, 4, 8),
		{ 0, 5, -12 }
	};
	objects[n_objects++] = (Object) {
		s_pyramid(8, 4, 8),
		{ -8, 0, -10 }
	};
	objects[n_objects++] = (Object) {
		s_pyramid(8, 4, 8),
		{ 8, -3, -20 }
	};

	loop(15, draw);

	return 0;
}
