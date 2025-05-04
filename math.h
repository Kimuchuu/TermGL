#ifndef LIB3C_MATH_H
#define LIB3C_MATH_H

typedef struct { float x, y; } Vec2f;
typedef struct { float x, y, z; } Vec3f;
typedef struct { float x, y, z, w; } Vec4f;

typedef struct { float m[3][3]; } Matrix3x3f;
typedef struct { float m[4][4]; } Matrix4x4f;

float radians(float);

Vec3f vec3f_add(Vec3f v1, Vec3f v2);
float vec3f_dot(Vec3f v1, Vec3f v2);
Vec3f vec3f_cross(Vec3f v1, Vec3f v2);
Vec3f vec3f_mult(Vec3f v1, Vec3f v2);
Vec3f vec3f_normalize(Vec3f);
Vec3f vec3f_reflect(Vec3f v, Vec3f n);
Vec3f vec3f_scale(Vec3f v1, float s);
Vec3f vec3f_sub(Vec3f v1, Vec3f v2);
Vec4f vec3f_to_vec4f(Vec3f *, float);

Vec3f vec4f_to_vec3f(Vec4f *);

Vec3f      mat3f_multv(Matrix3x3f *matrix, Vec3f *vector);
Matrix3x3f mat3f_transpose(Matrix3x3f *m);

Matrix4x4f mat4f_identity(float);
Matrix4x4f mat4f_inverse_transpose_affine(const Matrix4x4f*);
Matrix4x4f mat4f_mult(Matrix4x4f *m1, Matrix4x4f *m2);
Vec4f      mat4f_multv(Matrix4x4f *matrix, Vec4f *vector);
Matrix4x4f mat4f_translate(Vec3f);

Matrix4x4f look_at(Vec3f position, Vec3f target, Vec3f up);
Matrix4x4f perspective(float fov, float aspect_ratio, float near, float far);

#endif
