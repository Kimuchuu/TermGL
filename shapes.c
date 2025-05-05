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
	polygon.faces = malloc(3 * 6 * sizeof(int));
	polygon.n_faces = 6;

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

