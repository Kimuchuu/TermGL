#include "math.h"
#include <math.h>

//
// general
//

float radians(float degrees) {
	return degrees * M_PI / 180.f;
}


//
// scale
//

Vec2f vec2f_scale(Vec2f v1, float f) {
	v1.x *= f;
	v1.y *= f;
	return v1;
}

Vec2f vec2f_sub(Vec2f v1, Vec2f v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}


//
// vec3f
//

Vec3f vec3f_add(Vec3f v1, Vec3f v2) {
	Vec3f result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

Vec3f vec3f_cross(Vec3f v1, Vec3f v2) {
	Vec3f result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

float vec3f_dot(Vec3f v1, Vec3f v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec3f vec3f_mult(Vec3f v1, Vec3f v2) {
	return (Vec3f) { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

Vec3f vec3f_normalize(Vec3f v) {
	Vec3f result;
	float magnitude = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	result.x = v.x / magnitude;
	result.y = v.y / magnitude;
	result.z = v.z / magnitude;
	return result;
}

Vec3f vec3f_reflect(Vec3f v, Vec3f n) {
	return vec3f_sub(v, vec3f_scale(n, vec3f_dot(n, v) * 2.f));
}

Vec3f vec3f_scale(Vec3f v1, float s) {
	Vec3f result;
	result.x = v1.x * s;
	result.y = v1.y * s;
	result.z = v1.z * s;
	return result;
}

Vec3f vec3f_sub(Vec3f v1, Vec3f v2) {
	Vec3f result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

Vec4f vec3f_to_vec4f(Vec3f *v, float w) {
	Vec4f result = { v->x, v->y, v->z, w };
	return result;
}


//
// vec4f
//

Vec4f vec4f_add(Vec4f v1, Vec4f v2) {
	Vec4f result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	result.w = v1.w + v2.w;
	return result;
}

Vec4f vec4f_scale(Vec4f v1, float s) {
	Vec4f result;
	result.x = v1.x * s;
	result.y = v1.y * s;
	result.z = v1.z * s;
	result.w = v1.w * s;
	return result;
}

Vec4f vec4f_sub(Vec4f v1, Vec4f v2) {
	Vec4f result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	result.w = v1.w - v2.w;
	return result;
}

Vec3f vec4f_to_vec3f(Vec4f *v) {
	Vec3f result = { v->x, v->y, v->z };
	return result;
}


//
// mat3f
//


Vec3f mat3f_multv(Matrix3x3f *matrix, Vec3f *vector) {
	Vec3f result;
	result.x =
		matrix->m[0][0] * vector->x +
		matrix->m[0][1] * vector->y +
		matrix->m[0][2] * vector->z;
	result.y =
		matrix->m[1][0] * vector->x +
		matrix->m[1][1] * vector->y +
		matrix->m[1][2] * vector->z;
	result.z =
		matrix->m[2][0] * vector->x +
		matrix->m[2][1] * vector->y +
		matrix->m[2][2] * vector->z;
	return result;
}

Matrix3x3f mat3f_transpose(Matrix3x3f *m) {
	Matrix3x3f result;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = m->m[j][i];
		}
	}
	return result;
}


//
// mat4f
//

Matrix4x4f mat4f_identity(float f) {
	Matrix4x4f mat = {{
		{ f, 0, 0, 0 },
		{ 0, f, 0, 0 },
		{ 0, 0, f, 0 },
		{ 0, 0, 0, f }
	}};
	return mat;
}

Matrix4x4f mat4f_inverse_transpose_affine(const Matrix4x4f* matrix) {
	Matrix4x4f result;
	float det3x3;
	float inv_det3x3;

	float m00 = matrix->m[0][0], m01 = matrix->m[0][1], m02 = matrix->m[0][2];
	float m10 = matrix->m[1][0], m11 = matrix->m[1][1], m12 = matrix->m[1][2];
	float m20 = matrix->m[2][0], m21 = matrix->m[2][1], m22 = matrix->m[2][2];

	det3x3 = m00 * (m11 * m22 - m12 * m21)
		   - m01 * (m10 * m22 - m12 * m20)
		   + m02 * (m10 * m21 - m11 * m20);

	if (fabsf(det3x3) < 1e-6f) {
		return mat4f_identity(1.0f);
	}

	inv_det3x3 = 1.0f / det3x3;

	result.m[0][0] = (m11 * m22 - m12 * m21) * inv_det3x3;
	result.m[1][0] = (m02 * m21 - m01 * m22) * inv_det3x3;
	result.m[2][0] = (m01 * m12 - m02 * m11) * inv_det3x3;

	result.m[0][1] = (m12 * m20 - m10 * m22) * inv_det3x3;
	result.m[1][1] = (m00 * m22 - m02 * m20) * inv_det3x3;
	result.m[2][1] = (m02 * m10 - m00 * m12) * inv_det3x3;

	result.m[0][2] = (m10 * m21 - m11 * m20) * inv_det3x3;
	result.m[1][2] = (m01 * m20 - m00 * m21) * inv_det3x3;
	result.m[2][2] = (m00 * m11 - m01 * m10) * inv_det3x3;

	result.m[0][3] = result.m[1][3] = result.m[2][3] = 0.0f;
	result.m[3][0] = result.m[3][1] = result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4f mat4f_mult(Matrix4x4f *m1, Matrix4x4f *m2) {
	Matrix4x4f result = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += m1->m[i][k] * m2->m[k][j];
			}
		}
	}
	return result;
}

Vec4f mat4f_multv(Matrix4x4f *matrix, Vec4f *vector) {
	Vec4f result;
	result.x =
		matrix->m[0][0] * vector->x +
		matrix->m[0][1] * vector->y +
		matrix->m[0][2] * vector->z +
		matrix->m[0][3] * vector->w;
	result.y =
		matrix->m[1][0] * vector->x +
		matrix->m[1][1] * vector->y +
		matrix->m[1][2] * vector->z +
		matrix->m[1][3] * vector->w;
	result.z =
		matrix->m[2][0] * vector->x +
		matrix->m[2][1] * vector->y +
		matrix->m[2][2] * vector->z +
		matrix->m[2][3] * vector->w;
	result.w =
		matrix->m[3][0] * vector->x +
		matrix->m[3][1] * vector->y +
		matrix->m[3][2] * vector->z +
		matrix->m[3][3] * vector->w;
	return result;
}

Matrix4x4f mat4f_rotate_y(float angles) {
	float c = cosf(angles);
	float s = sinf(angles);
	Matrix4x4f result = {{
		{   c, 0.f,   s, 0.f },
		{ 0.f, 1.f, 0.f, 0.f },
		{  -s, 0.f,   c, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	}};
	return result;
}

Matrix4x4f mat4f_rotate_x(float angle) {
	float c = cosf(angle);
	float s = sinf(angle);
	Matrix4x4f result = {{
		{ 1.f, 0.f, 0.f, 0.f },
		{ 0.f,   c,  -s, 0.f },
		{ 0.f,   s,   c, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	}};
	return result;
}

Matrix4x4f mat4f_rotate_z(float angle) {
	float c = cosf(angle);
	float s = sinf(angle);
	Matrix4x4f result = {{
		{   c,  -s, 0.f, 0.f },
		{   s,   c, 0.f, 0.f },
		{ 0.f, 0.f, 1.f, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	}};
	return result;
}

Matrix4x4f mat4f_scale(Vec3f s) {
	Matrix4x4f result = {{
		{ s.x, 0.f, 0.f, 0.f },
		{ 0.f, s.y, 0.f, 0.f },
		{ 0.f, 0.f, s.z, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	}};
	return result;
}

Matrix4x4f mat4f_translate(Vec3f v) {
	Matrix4x4f result = {{
		{ 1.f, 0.f, 0.f, v.x },
		{ 0.f, 1.f, 0.f, v.y },
		{ 0.f, 0.f, 1.f, v.z },
		{ 0.f, 0.f, 0.f, 1.f }
	}};
	return result;
}


//
// utils
//

Matrix4x4f look_at(Vec3f eye, Vec3f target, Vec3f up) {
	Vec3f camera_direction = vec3f_normalize(vec3f_sub(eye, target));
	Vec3f camera_right = vec3f_normalize(vec3f_cross(up, camera_direction));
	Vec3f camera_up = vec3f_cross(camera_direction, camera_right);
	Matrix4x4f rotation = {{
		{     camera_right.x,     camera_right.y,     camera_right.z, 0.f },
		{        camera_up.x,        camera_up.y,        camera_up.z, 0.f },
		{ camera_direction.x, camera_direction.y, camera_direction.z, 0.f },
		{                0.f,                0.f,                0.f, 1.f }
	}};
	Matrix4x4f translation = {{
		{ 1.f, 0.f, 0.f, -eye.x },
		{ 0.f, 1.f, 0.f, -eye.y },
		{ 0.f, 0.f, 1.f, -eye.z },
		{ 0.f, 0.f, 0.f,    1.f }
	}};
	return mat4f_mult(&rotation, &translation);
}

Matrix4x4f perspective(float fov, float aspect_ratio, float near, float far) {
    float tangent = tan(radians(fov / 2.0f));
    float top = near * tangent;
    float right = top * aspect_ratio;
    Matrix4x4f matrix = {};
    matrix.m[0][0] = near / right;
    matrix.m[1][1] = near / top;
    matrix.m[2][2] = -(far + near) / (far - near);
    matrix.m[2][3] = -(2 * far * near) / (far - near);
    matrix.m[3][2] = -1;
    return matrix;
}

