#include "glmath.h"

#include <math.h>

void normalize(vec3 *v)
{
    float d = sqrtf((v->x) * (v->x) + (v->y) * (v->y) + (v->z) * (v->z));
    v->x /= d; v->y /= d; v->z /= d;
}

void identity(mat4 *m)
{
    m->value[0]  = 1.0f; m->value[1]  = 0.0f; m->value[2]  = 0.0f; m->value[3]  = 0.0f;
    m->value[4]  = 0.0f; m->value[5]  = 1.0f; m->value[6]  = 0.0f; m->value[7]  = 0.0f;
    m->value[8]  = 0.0f; m->value[9]  = 0.0f; m->value[10] = 1.0f; m->value[11] = 0.0f;
    m->value[12] = 0.0f; m->value[13] = 0.0f; m->value[14] = 0.0f; m->value[15] = 1.0f;
}

void translate(mat4 *m, vec3 *v)
{
    m->value[0]  = 1.0f; m->value[1]  = 0.0f; m->value[2]  = 0.0f; m->value[3]  = 0.0f;
    m->value[4]  = 0.0f; m->value[5]  = 1.0f; m->value[6]  = 0.0f; m->value[7]  = 0.0f;
    m->value[8]  = 0.0f; m->value[9]  = 0.0f; m->value[10] = 1.0f; m->value[11] = 0.0f;
    m->value[12] = v->x; m->value[13] = v->y; m->value[14] = v->z; m->value[15] = 1.0f;
}

void translate_v2(mat4 *m, vec2 *v)
{
    m->value[0]  = 1.0f; m->value[1]  = 0.0f; m->value[2]  = 0.0f; m->value[3]  = 0.0f;
    m->value[4]  = 0.0f; m->value[5]  = 1.0f; m->value[6]  = 0.0f; m->value[7]  = 0.0f;
    m->value[8]  = 0.0f; m->value[9]  = 0.0f; m->value[10] = 1.0f; m->value[11] = 0.0f;
    m->value[12] = v->x; m->value[13] = v->y; m->value[14] = 0.0f; m->value[15] = 1.0f;
}

void add_v3(vec3 *v, vec3 *v1, vec3 *v2)
{
    v->x = v1->x + v2->x;
    v->y = v1->y + v2->y;
    v->z = v1->z + v2->z;
}

void add_v2(vec2 *v, vec2 *v1, vec2 *v2)
{
    v->x = v1->x + v2->x;
    v->y = v1->y + v2->y;
}

void subtract_v2(vec2 *v, vec2 *v1, vec2 *v2)
{
    v->x = v1->x - v2->x;
    v->y = v1->y - v2->y;
}

void rotate(mat4 *matrix, vec3 *axis, float angle)
{
    normalize(axis);
    float s = sinf(angle);
    float c = cosf(angle);
    float m = 1.0f - c;
    matrix->value[0] = m * axis->x * axis->x + c;
    matrix->value[1] = m * axis->x * axis->y - axis->z * s;
    matrix->value[2] = m * axis->z * axis->x + axis->y * s;
    matrix->value[3] = 0.0f;
    matrix->value[4] = m * axis->x * axis->y + axis->z * s;
    matrix->value[5] = m * axis->y * axis->y + c;
    matrix->value[6] = m * axis->y * axis->z - axis->x * s;
    matrix->value[7] = 0.0f;
    matrix->value[8] = m * axis->z * axis->x - axis->y * s;
    matrix->value[9] = m * axis->y * axis->z + axis->x * s;
    matrix->value[10] = m * axis->z * axis->z + c;
    matrix->value[11] = 0.0f;
    matrix->value[12] = 0.0f;
    matrix->value[13] = 0.0f;
    matrix->value[14] = 0.0f;
    matrix->value[15] = 1.0f;
}

void multiply(mat4 *m, mat4 *m1, mat4 *m2)
{
    mat4 result;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            int index = c * 4 + r;
            float total = 0;
            for (int i = 0; i < 4; i++) {
                int p = i * 4 + r;
                int q = c * 4 + i;
                total += m1->value[p] * m2->value[q];
            }
            result.value[index] = total;
        }
    }
    *m = result;
}

void multiply_v3f(vec3 *v, vec3 *v1, float v2)
{
    v->x = v1->x * v2;
    v->y = v1->y * v2;
    v->z = v1->z * v2;
}

void multiply_v2f(vec2 *v, vec2 *v1, float v2)
{
    v->x = v1->x * v2;
    v->y = v1->y * v2;
}

void frustum(mat4 *m, float left, float right, float bottom, float top, float znear, float zfar)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0f * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    m->value[0] = temp / temp2;
    m->value[1] = 0.0f;
    m->value[2] = 0.0f;
    m->value[3] = 0.0f;
    m->value[4] = 0.0f;
    m->value[5] = temp / temp3;
    m->value[6] = 0.0f;
    m->value[7] = 0.0f;
    m->value[8] = (right + left) / temp2;
    m->value[9] = (top + bottom) / temp3;
    m->value[10] = (-zfar - znear) / temp4;
    m->value[11] = -1.0f;
    m->value[12] = 0.0f;
    m->value[13] = 0.0f;
    m->value[14] = (-temp * zfar) / temp4;
    m->value[15] = 0.0f;
}

void perspective(mat4 *m, float fov, float aspect, float znear, float zfar)
{
    float ymax, xmax;
    ymax = znear * tanf(fov * PI / 360.0f);
    xmax = ymax * aspect;
    frustum(m, -xmax, xmax, -ymax, ymax, znear, zfar);
}

void ortho(mat4 *m, float left, float right, float bottom, float top, float near, float far)
{
    m->value[0] = 2 / (right - left);
    m->value[1] = 0;
    m->value[2] = 0;
    m->value[3] = 0;
    m->value[4] = 0;
    m->value[5] = 2 / (top - bottom);
    m->value[6] = 0;
    m->value[7] = 0;
    m->value[8] = 0;
    m->value[9] = 0;
    m->value[10] = -2 / (far - near);
    m->value[11] = 0;
    m->value[12] = -(right + left) / (right - left);
    m->value[13] = -(top + bottom) / (top - bottom);
    m->value[14] = -(far + near) / (far - near);
    m->value[15] = 1;
}
