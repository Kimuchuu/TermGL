#include <bits/time.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include "../3d.h"
#include "../simple3d.h"
#include "../shapes.h"
#include "../math.h"
#include "../window.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

typedef struct {
	Polygon polygon;
	Vec3f position;
} Object;

typedef struct {
	Matrix4x4f *m_model;
	Matrix4x4f *m_view;
	Matrix4x4f *m_projection;
} ShaderData;


static double near = 0.1;
static double far = 100;
static Camera camera = {
	.position = { 0.0, 0.0, 3.0 },
	.target = { 0.0, 0.0, 0.0 },
	.up = { 0.0, 1.0, 0.0 },
	.fov_y = 45
};
static Matrix4x4f m_view;
static Matrix4x4f m_projection;
static Object object;
static int image_width, image_height, bpp;
static stbi_uc *image;

Pixel fragment_shader(FragmentShaderInput *input, ShaderAttributes *in, void *data) {
	Vec4f uv = in->attributes[0].value.vec4f;

	int x = (int)(uv.x * (image_width - 1));
	int y = (image_height - 1) - (int)(uv.y * (image_height - 1));
	int index = (y * image_width + x) * bpp;
	float r = image[index + 0] / 255.0f;
	float g = image[index + 1] / 255.0f;
	float b = image[index + 2] / 255.0f;
	float a = image[index + 3];

	Pixel fragment;
	fragment.c       = a == 0 ? 'o' : '@';
	fragment.color.x = a == 0 ?  1  :  r ;
	fragment.color.y = a == 0 ?  1  :  g ;
	fragment.color.z = a == 0 ?  1  :  b ;
	return fragment;
}

VertexShaderOutput vertex_shader(Point3D *point, void *data, ShaderAttributes *out) {
	ShaderData *uniform = data;
	Vec4f world_pos = { point->position.x, point->position.y, point->position.z, 1.f };
	world_pos = mat4f_multv(uniform->m_model, &world_pos);
	Vec4f point_view = mat4f_multv(uniform->m_view, &world_pos);
	Vec4f clip_pos = mat4f_multv(uniform->m_projection, &point_view);

	out->n = 1;
	out->attributes[0].type = VEC4F;
	out->attributes[0].value.vec4f = (Vec4f){ point->uv.x, point->uv.y, 0.f, 1.f };

	VertexShaderOutput output = { .clip_pos = clip_pos };
	return output;
}


void draw(unsigned int frame, double time, double delta) {
	static float degrees = 0.f;
	degrees += (delta / 1000.f) * 90.f;

	Matrix4x4f translate = mat4f_translate(object.position);
	Matrix4x4f rotate = mat4f_rotate_y(radians(degrees));
	Matrix4x4f model = mat4f_identity(1);
	model = mat4f_mult(&model, &translate);
	model = mat4f_mult(&model, &rotate);

	ShaderData data = { &model, &m_view, &m_projection };
	print_polygon(&object.polygon, &data, vertex_shader, fragment_shader);
}

void on_terminate(int code) {
	cleanup_window();
	exit(code);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, on_terminate);

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int cols = w.ws_col;
	int rows = w.ws_row;
	int width = w.ws_xpixel;
	int height = w.ws_ypixel;
	if (width == 0 || height == 0) {
		width = cols;
		height = rows * 2;
	}
	init_window(cols, rows);
	init_3d();
	init_camera(&camera);
	m_view = look_at(camera.position, camera.target, camera.up);
	m_projection = perspective(camera.fov_y,  width / (float) height, near, far);

	image = stbi_load("./assets/awesomeface.png", &image_width, &image_height, &bpp, 0);
	if (image == NULL) {
		fprintf(stderr, "Couldn't load image: %s\n", stbi_failure_reason());
		exit(1);
	}

	object.polygon = s_cube(8, 8, 8);
	object.position = (Vec3f){ 0, 0, -12 };
	Vec2f uv[8] = {
		{ 0, 0 }, { 1, 0 }, { 1, 0 }, { 0, 0 },
		{ 0, 1 }, { 1, 1 }, { 1, 1 }, { 0, 1 }
	};
	for (int i = 0; i < 8; i++) {
		object.polygon.points[i].uv = uv[i];
	}

	loop(15, draw);

	return 0;
}
