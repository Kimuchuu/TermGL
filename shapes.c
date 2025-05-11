#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "shapes.h"

#define EPSILON 1e-6

int set_point(Point3D *triangles, int index, Point3D p) {
	triangles[index] = p;
	return index;
}

void add_vertex_normals(Polygon *polygon) {
	Point3D *points = polygon->points;

	for (int i = 0; i < polygon->n_points; i++) {
		points[i].normal = (Vec3f) { 0.f, 0.f, 0.f };
	}

	for (int i = 0; i < polygon->n_faces; i++) {
		int i0 = polygon->faces[i * 3];
		int i1 = polygon->faces[i * 3 + 1];
		int i2 = polygon->faces[i * 3 + 2];
		Vec3f p0 = points[i0].position;
		Vec3f p1 = points[i1].position;
		Vec3f p2 = points[i2].position;
		Vec3f edge1 = vec3f_sub(p1, p0);
		Vec3f edge2 = vec3f_sub(p2, p0);
		Vec3f face_normal = vec3f_cross(edge1, edge2);

		Vec3f edge1_norm = vec3f_normalize(vec3f_sub(p1, p0));
		Vec3f edge2_norm = vec3f_normalize(vec3f_sub(p2, p0));
		Vec3f edge_p1_p2_norm = vec3f_normalize(vec3f_sub(p2, p1));
		float angle0 = acosf(fmaxf(-1.0f, fminf(1.0f, vec3f_dot(edge1_norm, edge2_norm))));
		float angle1 = acosf(fmaxf(-1.0f, fminf(1.0f, vec3f_dot(vec3f_scale(edge1_norm, -1.f), edge_p1_p2_norm))));
		float angle2 = acosf(fmaxf(-1.0f, fminf(1.0f, vec3f_dot(vec3f_scale(edge2_norm, -1.f), vec3f_scale(edge_p1_p2_norm, -1.f)))));

		points[i0].normal = vec3f_add(points[i0].normal, vec3f_scale(face_normal, angle0));
		points[i1].normal = vec3f_add(points[i1].normal, vec3f_scale(face_normal, angle1));
		points[i2].normal = vec3f_add(points[i2].normal, vec3f_scale(face_normal, angle2));
	}

	for (int i = 0; i < polygon->n_points; i++) {
		float normal_length = sqrtf(vec3f_dot(points[i].normal, points[i].normal));
		points[i].normal = normal_length > EPSILON
						 ? vec3f_normalize(points[i].normal)
						 : (Vec3f) { 0.f, 0.f, 0.f };
	}
}

Polygon s_circle(float radius, int steps) {
	Polygon polygon;
	polygon.n_points = steps + 1;
	polygon.points = malloc(polygon.n_points * sizeof(Point3D));
	polygon.n_faces = steps;
	polygon.faces = malloc(3 * polygon.n_faces * sizeof(int));

	float b = (M_PI * 2.0) / steps;
	float b_cos = cosf(b);
	float b_sin = sinf(b);

	polygon.points[0] = (Point3D) {
		{ 0, 0, 0 },
		{ 255, 200, 30 }
	};
	polygon.points[1] = (Point3D) {
		{ 0, radius, 0 },
		{ 0, 0, 255 / steps }
	};

	Vec3f last_pos;
	for (int i = 2; i < polygon.n_points; i++) {
		last_pos = polygon.points[i - 1].position;
		polygon.points[i].position = (Vec3f) {
			last_pos.x * b_cos - last_pos.y * b_sin,
			last_pos.y * b_cos + last_pos.x * b_sin,
			0
		};
		polygon.points[i].color = (Color){ 0, 0, 255/steps * i};
	}

	int order[polygon.n_faces * 3];
	for (int i = 0; i < polygon.n_faces; i++) {
		order[i * 3] = 0;
		order[i * 3 + 1] = i + 1;
		order[i * 3 + 2] = ((i + 1) % polygon.n_faces) + 1;
	}

	memcpy(polygon.faces, order, sizeof(order));
	add_vertex_normals(&polygon);

	return polygon;
}

Polygon s_cube(float width, float height, float depth) {
	Polygon polygon;
	polygon.n_points = 8;
	polygon.points = malloc(polygon.n_points * sizeof(Point3D));
	polygon.n_faces = 12;
	polygon.faces = malloc(3 * polygon.n_faces * sizeof(int));

	float wh = width / 2;
	float hh = height / 2;
	float dh = depth / 2;

	int i = 0;

	int bfl = set_point(polygon.points, i++, (Point3D) {
		{ -wh, -hh, dh },
		{ 255, 0, 0}
	});
	int bfr = set_point(polygon.points, i++, (Point3D) {
		{ wh, -hh, dh },
		{ 0, 255, 0}
	});
	int bbl = set_point(polygon.points, i++, (Point3D) {
		{ -wh, -hh, -dh },
		{ 255, 0, 0}
	});
	int bbr = set_point(polygon.points, i++, (Point3D) {
		{ wh, -hh, -dh },
		{ 0, 255, 0}
	});
	int tfl = set_point(polygon.points, i++, (Point3D) {
		{ -wh, hh, dh },
		{ 255, 0, 0}
	});
	int tfr = set_point(polygon.points, i++, (Point3D) {
		{ wh, hh, dh },
		{ 0, 255, 0}
	});
	int tbl = set_point(polygon.points, i++, (Point3D) {
		{ -wh, hh, -dh },
		{ 255, 0, 0}
	});
	int tbr = set_point(polygon.points, i++, (Point3D) {
		{ wh, hh, -dh },
		{ 0, 255, 0}
	});

	int order[12 * 3] = {
		// FRONT
		bfl, bfr, tfl,
		bfr, tfr, tfl,

		// RIGHT
		bfr, bbr, tfr,
		bbr, tbr, tfr,

		// BACK
		bbr, bbl, tbr,
		bbl, tbl, tbr,

		// LEFT
		bbl, bfl, tbl,
		bfl, tfl, tbl,

		// TOP
		tfl, tfr, tbl,
		tfr, tbr, tbl,

		// BOTTOM
		bbl, bbr, bfl,
		bbr, bfr, bfl,
	};

	memcpy(polygon.faces, order, sizeof(order));
	add_vertex_normals(&polygon);

	return polygon;
}

Polygon s_pyramid(float width, float height, float depth) {
	Polygon polygon;
	polygon.n_points = 5;
	polygon.points = malloc(polygon.n_points * sizeof(Point3D));
	polygon.n_faces = 6;
	polygon.faces = malloc(3 * polygon.n_faces * sizeof(int));

	float wh = width / 2;
	float hh = height / 2;
	float dh = depth / 2;

	int i = 0;

	int fl = set_point(polygon.points, i++, (Point3D) {
		{ -wh, -hh, dh },
		{ 255, 0, 0}
	});
	int fr = set_point(polygon.points, i++, (Point3D) {
		{ wh, -hh, dh },
		{ 0, 255, 0}
	});
	int bl = set_point(polygon.points, i++, (Point3D) {
		{ -wh, -hh, -dh },
		{ 255, 0, 0}
	});
	int br = set_point(polygon.points, i++, (Point3D) {
		{ wh, -hh, -dh },
		{ 0, 255, 0}
	});
	int top = set_point(polygon.points, i++, (Point3D) {
		{ 0, hh, 0 },
		{ 0, 0, 255 }
	});

	int order[6 * 3] = {
		fl, bl,  br,
		fl, br,  fr,
		fl, fr, top,
		br, bl, top,
		bl, fl, top,
		fr, br, top,
	};

	memcpy(polygon.faces, order, sizeof(order));
	add_vertex_normals(&polygon);

	return polygon;
}

Polygon s_sphere(float radius, int stacks, int sectors) {
	Polygon polygon;
	polygon.n_points = (stacks + 1) * (sectors + 1);
	polygon.points = malloc(polygon.n_points * sizeof(Point3D));
	polygon.n_faces = (stacks - 1) * sectors * 2;
	polygon.faces = malloc(3 * polygon.n_faces * sizeof(int));

	float x, y, z, xy;
	float nx, ny, nz, length_inv = 1.0f / radius;

	float sector_step = 2 * M_PI / sectors;
	float stack_step = M_PI / stacks;
	float sector_angle, stack_angle;

	int points_index = 0;
	for (int i = 0; i <= stacks; i++) {
		stack_angle = M_PI / 2 - i * stack_step;
		xy = radius * cosf(stack_angle);
		z = radius * sinf(stack_angle);

		for (int j = 0; j <= sectors; j++) {
			sector_angle = j * sector_step;

			x = xy * cosf(sector_angle);
			y = xy * sinf(sector_angle);

			nx = x * length_inv;
			ny = y * length_inv;
			nz = z * length_inv;

			polygon.points[points_index++] = (Point3D) {
				{ x, y, z },
				{ (255 / stacks) * (stacks - i), (255 / stacks) * i, 40 },
				{ nx, ny, nz }
			};
		}
	}

	int order[polygon.n_faces * 3];
	int order_index = 0;
	int k1, k2;
	for (int i = 0; i < stacks; i++) {
		k1 = i * (sectors + 1);
		k2 = k1 + sectors + 1;

		for (int j = 0; j < sectors; j++, k1++, k2++) {
			if (i != 0) {
				order[order_index++] = k1;
				order[order_index++] = k2;
				order[order_index++] = k1 + 1;
			}

			if (i != (stacks - 1)) {
				order[order_index++] = k1 + 1;
				order[order_index++] = k2;
				order[order_index++] = k2 + 1;
			}
		}
	}

	memcpy(polygon.faces, order, sizeof(order));

	return polygon;
}

