#include <string.h>
#include <math.h>
#include "math_utils.h"

/*
 * Note: matrices are row-major.
 */

static inline float sgn(float a)
{
	if (a > 0.0f)
		return 1.0f;
	if (a < 0.0f)
		return -1.0f;
	return 0.0f;
}

void vector3f_init(vector3f *v, float x, float y, float z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

void vector3f_copy(vector3f *dst, const vector3f *src)
{
	dst->x = src->x;
	dst->y = src->y;
	dst->z = src->z;
}

void vector3f_add(vector3f *v1, const vector3f *v2)
{
	v1->x += v2->x;
	v1->y += v2->y;
	v1->z += v2->z;
}

void vector3f_scalar_mult(vector3f *v, float a)
{
	v->x *= a;
	v->y *= a;
	v->z *= a;
}

void vector3f_add_mult(vector3f *v, const vector3f *u, float a)
{
	v->x += u->x * a;
	v->y += u->y * a;
	v->z += u->z * a;
}

void vector3f_opposite(vector3f *v1, const vector3f *v0)
{
	v1->x = -v0->x;
	v1->y = -v0->y;
	v1->z = -v0->z;
}

float vector3f_dot_product(const vector3f *v1, const vector3f *v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

void vector3f_cross_product(vector3f *w, const vector3f *u, const vector3f *v)
{
	w->x = u->y * v->z - u->z * v->y;
	w->y = u->z * v->x - u->x * v->z;
	w->z = u->x * v->y - u->y * v->x;
}

void vector3f_matrix4x4_mult(vector3f *u, const matrix4x4 m, const vector3f *v, float w)
{
	u->x = m[0][0] * v->x + m[0][1] * v->y + m[0][2] * v->z + m[0][3] * w;
	u->y = m[1][0] * v->x + m[1][1] * v->y + m[1][2] * v->z + m[1][3] * w;
	u->z = m[2][0] * v->x + m[2][1] * v->y + m[2][2] * v->z + m[2][3] * w;
}

void vector4f_init(vector4f *v, float x, float y, float z, float w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void vector4f_scalar_mult_dest(vector4f *u, const vector4f *v, float a)
{
	u->x = v->x * a;
	u->y = v->y * a;
	u->z = v->z * a;
	u->w = v->w * a;
}

float vector4f_dot_product(const vector4f *v1, const vector4f *v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

void vector4f_matrix4x4_mult(vector4f *u, const matrix4x4 m, const vector4f *v)
{
	u->x = m[0][0] * v->x + m[0][1] * v->y + m[0][2] * v->z + m[0][3] * v->w;
	u->y = m[1][0] * v->x + m[1][1] * v->y + m[1][2] * v->z + m[1][3] * v->w;
	u->z = m[2][0] * v->x + m[2][1] * v->y + m[2][2] * v->z + m[2][3] * v->w;
	u->w = m[3][0] * v->x + m[3][1] * v->y + m[3][2] * v->z + m[3][3] * v->w;
}

void matrix3x3_identity(matrix3x3 m)
{
	m[0][1] = m[0][2] = 0.0f;
	m[1][0] = m[1][2] = 0.0f;
	m[2][0] = m[2][1] = 0.0f;
	m[0][0] = m[1][1] = m[2][2] = 1.0f;
}

void matrix3x3_from_matrix4x4(matrix3x3 dst, const matrix4x4 src)
{
	int i, j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			dst[i][j] = src[i][j];
	}
}

void matrix4x4_identity(matrix4x4 m)
{
	m[0][1] = m[0][2] = m[0][3] = 0.0f;
	m[1][0] = m[1][2] = m[1][3] = 0.0f;
	m[2][0] = m[2][1] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void matrix4x4_copy(matrix4x4 dst, const matrix4x4 src)
{
	memcpy(dst, src, sizeof(matrix4x4));
}

void matrix4x4_multiply(matrix4x4 dst, const matrix4x4 src1, const matrix4x4 src2)
{
	int i, j, k;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			dst[i][j] = 0.0f;
			for (k = 0; k < 4; k++)
				dst[i][j] += src1[i][k] * src2[k][j];
		}
	}
}

void matrix4x4_init_translation(matrix4x4 m, float x, float y, float z)
{
	matrix4x4_identity(m);

	m[0][3] = x;
	m[1][3] = y;
	m[2][3] = z;
}

void matrix4x4_init_translation_vector3f(matrix4x4 m, const vector3f *v)
{
	matrix4x4_identity(m);

	m[0][3] = v->x;
	m[1][3] = v->y;
	m[2][3] = v->z;
}

void matrix4x4_translate(matrix4x4 m, float x, float y, float z)
{
	matrix4x4 m1, m2;

	matrix4x4_init_translation(m1, x, y, z);
	matrix4x4_multiply(m2, m, m1);
	matrix4x4_copy(m, m2);
}

void matrix4x4_init_scaling(matrix4x4 m, float scale_x, float scale_y, float scale_z)
{
	matrix4x4_identity(m);

	m[0][0] = scale_x;
	m[1][1] = scale_y;
	m[2][2] = scale_z;
}

void matrix4x4_scale(matrix4x4 m, float scale_x, float scale_y, float scale_z)
{
	matrix4x4 m1, m2;

	matrix4x4_init_scaling(m1, scale_x, scale_y, scale_z);
	matrix4x4_multiply(m2, m, m1);
	matrix4x4_copy(m, m2);
}

void matrix4x4_reflect_origin(matrix4x4 m)
{
	matrix4x4_scale(m, -1.0f, -1.0f, -1.0f);
}

void matrix4x4_transpose(matrix4x4 out, const matrix4x4 m)
{
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			out[i][j] = m[j][i];
	}
}

int matrix4x4_invert(matrix4x4 out, const matrix4x4 m)
{
	int i, j;
	float det;
	matrix4x4 inv;

	inv[0][0] = m[1][1]  * m[2][2] * m[3][3] -
		m[1][1]  * m[3][2] * m[2][3] -
		m[1][2]  * m[2][1]  * m[3][3] +
		m[1][2]  * m[3][1]  * m[2][3] +
		m[1][3] * m[2][1]  * m[3][2] -
		m[1][3] * m[3][1]  * m[2][2];

	inv[0][1] = -m[0][1]  * m[2][2] * m[3][3] +
		m[0][1]  * m[3][2] * m[2][3] +
		m[0][2]  * m[2][1]  * m[3][3] -
		m[0][2]  * m[3][1]  * m[2][3] -
		m[0][3] * m[2][1]  * m[3][2] +
		m[0][3] * m[3][1]  * m[2][2];

	inv[0][2] = m[0][1]  * m[1][2] * m[3][3] -
		m[0][1]  * m[3][2] * m[1][3] -
		m[0][2]  * m[1][1] * m[3][3] +
		m[0][2]  * m[3][1] * m[1][3] +
		m[0][3] * m[1][1] * m[3][2] -
		m[0][3] * m[3][1] * m[1][2];

	inv[0][3] = -m[0][1]  * m[1][2] * m[2][3] +
		m[0][1]  * m[2][2] * m[1][3] +
		m[0][2]  * m[1][1] * m[2][3] -
		m[0][2]  * m[2][1] * m[1][3] -
		m[0][3] * m[1][1] * m[2][2] +
		m[0][3] * m[2][1] * m[1][2];

	inv[1][0] = -m[1][0]  * m[2][2] * m[3][3] +
		m[1][0]  * m[3][2] * m[2][3] +
		m[1][2]  * m[2][0] * m[3][3] -
		m[1][2]  * m[3][0] * m[2][3] -
		m[1][3] * m[2][0] * m[3][2] +
		m[1][3] * m[3][0] * m[2][2];

	inv[1][1] = m[0][0]  * m[2][2] * m[3][3] -
		m[0][0]  * m[3][2] * m[2][3] -
		m[0][2]  * m[2][0] * m[3][3] +
		m[0][2]  * m[3][0] * m[2][3] +
		m[0][3] * m[2][0] * m[3][2] -
		m[0][3] * m[3][0] * m[2][2];

	inv[1][2] = -m[0][0]  * m[1][2] * m[3][3] +
		m[0][0]  * m[3][2] * m[1][3] +
		m[0][2]  * m[1][0] * m[3][3] -
		m[0][2]  * m[3][0] * m[1][3] -
		m[0][3] * m[1][0] * m[3][2] +
		m[0][3] * m[3][0] * m[1][2];

	inv[1][3] = m[0][0]  * m[1][2] * m[2][3] -
		m[0][0]  * m[2][2] * m[1][3] -
		m[0][2]  * m[1][0] * m[2][3] +
		m[0][2]  * m[2][0] * m[1][3] +
		m[0][3] * m[1][0] * m[2][2] -
		m[0][3] * m[2][0] * m[1][2];

	inv[2][0] = m[1][0]  * m[2][1] * m[3][3] -
		m[1][0]  * m[3][1] * m[2][3] -
		m[1][1]  * m[2][0] * m[3][3] +
		m[1][1]  * m[3][0] * m[2][3] +
		m[1][3] * m[2][0] * m[3][1] -
		m[1][3] * m[3][0] * m[2][1];

	inv[2][1] = -m[0][0]  * m[2][1] * m[3][3] +
		m[0][0]  * m[3][1] * m[2][3] +
		m[0][1]  * m[2][0] * m[3][3] -
		m[0][1]  * m[3][0] * m[2][3] -
		m[0][3] * m[2][0] * m[3][1] +
		m[0][3] * m[3][0] * m[2][1];

	inv[2][2] = m[0][0]  * m[1][1] * m[3][3] -
		m[0][0]  * m[3][1] * m[1][3] -
		m[0][1]  * m[1][0] * m[3][3] +
		m[0][1]  * m[3][0] * m[1][3] +
		m[0][3] * m[1][0] * m[3][1] -
		m[0][3] * m[3][0] * m[1][1];

	inv[2][3] = -m[0][0]  * m[1][1] * m[2][3] +
		m[0][0]  * m[2][1] * m[1][3] +
		m[0][1]  * m[1][0] * m[2][3] -
		m[0][1]  * m[2][0] * m[1][3] -
		m[0][3] * m[1][0] * m[2][1] +
		m[0][3] * m[2][0] * m[1][1];

	inv[3][0] = -m[1][0] * m[2][1] * m[3][2] +
		m[1][0] * m[3][1] * m[2][2] +
		m[1][1] * m[2][0] * m[3][2] -
		m[1][1] * m[3][0] * m[2][2] -
		m[1][2] * m[2][0] * m[3][1] +
		m[1][2] * m[3][0] * m[2][1];

	inv[3][1] = m[0][0] * m[2][1] * m[3][2] -
		m[0][0] * m[3][1] * m[2][2] -
		m[0][1] * m[2][0] * m[3][2] +
		m[0][1] * m[3][0] * m[2][2] +
		m[0][2] * m[2][0] * m[3][1] -
		m[0][2] * m[3][0] * m[2][1];

	inv[3][2] = -m[0][0] * m[1][1] * m[3][2] +
		m[0][0] * m[3][1] * m[1][2] +
		m[0][1] * m[1][0] * m[3][2] -
		m[0][1] * m[3][0] * m[1][2] -
		m[0][2] * m[1][0] * m[3][1] +
		m[0][2] * m[3][0] * m[1][1];

	inv[3][3] = m[0][0] * m[1][1] * m[2][2] -
		m[0][0] * m[2][1] * m[1][2] -
		m[0][1] * m[1][0] * m[2][2] +
		m[0][1] * m[2][0] * m[1][2] +
		m[0][2] * m[1][0] * m[2][1] -
		m[0][2] * m[2][0] * m[1][1];

	det = m[0][0] * inv[0][0] + m[1][0] * inv[0][1] + m[2][0] * inv[0][2] + m[3][0] * inv[0][3];

	if (det == 0)
		return 0;

	det = 1.0 / det;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			out[i][j] = inv[i][j] * det;
	}

	return 1;
}

void matrix4x4_get_x_axis(const matrix4x4 m, vector3f *x_axis)
{
	x_axis->x = m[0][0];
	x_axis->y = m[0][1];
	x_axis->z = m[0][2];
}

void matrix4x4_get_y_axis(const matrix4x4 m, vector3f *y_axis)
{
	y_axis->x = m[1][0];
	y_axis->y = m[1][1];
	y_axis->z = m[1][2];
}

void matrix4x4_get_z_axis(const matrix4x4 m, vector3f *z_axis)
{
	z_axis->x = m[2][0];
	z_axis->y = m[2][1];
	z_axis->z = m[2][2];
}

void matrix4x4_init_orthographic(matrix4x4 m, float left, float right, float bottom, float top, float near, float far)
{
	m[0][0] = 2.0f / (right - left);
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = -(right + left) / (right - left);

	m[1][0] = 0.0f;
	m[1][1] = 2.0f / (top - bottom);
	m[1][2] = 0.0f;
	m[1][3] = -(top + bottom) / (top - bottom);

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = -2.0f / (far - near);
	m[2][3] = -(far + near) / (far - near);

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void matrix4x4_init_frustum(matrix4x4 m, float left, float right, float bottom, float top, float near, float far)
{
	m[0][0] = (2.0f * near) / (right - left);
	m[0][1] = 0.0f;
	m[0][2] = (right + left) / (right - left);
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = (2.0f * near) / (top - bottom);
	m[1][2] = (top + bottom) / (top - bottom);
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = -(far + near) / (far - near);
	m[2][3] = (-2.0f * far * near) / (far - near);

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = -1.0f;
	m[3][3] = 0.0f;
}

void matrix3x3_normal_matrix(matrix3x3 out, const matrix4x4 m)
{
	matrix4x4 m1, m2;

	matrix4x4_invert(m1, m);
	matrix4x4_transpose(m2, m1);
	matrix3x3_from_matrix4x4(out, m2);
}

// Code from http://aras-p.info/texts/obliqueortho.html
// and http://www.terathon.com/code/oblique.php
void matrix4x4_oblique_near_plane(matrix4x4 projection, const vector4f *clip_plane)
{
	vector4f v;
	vector4f q;
	vector4f c;
	matrix4x4 proj_inv;

	vector4f_init(&v, sgn(clip_plane->x), sgn(clip_plane->y), 1.0f, 1.0f);

	matrix4x4_invert(proj_inv, projection);

	vector4f_matrix4x4_mult(&q, proj_inv, &v);

	vector4f_scalar_mult_dest(&c, clip_plane, 2.0f / vector4f_dot_product(clip_plane, &q));

	// third row = clip plane - fourth row
	projection[2][0] = c.x - projection[3][0];
	projection[2][1] = c.y - projection[3][1];
	projection[2][2] = c.z - projection[3][2];
	projection[2][3] = c.w - projection[3][3];
}
