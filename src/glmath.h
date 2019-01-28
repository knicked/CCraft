#ifndef GLMATH_H
#define GLMATH_H

#define PI 3.14159265359f
#define DEGREES(radians) ((radians) * 180.0f / PI)
#define RADIANS(degrees) ((degrees) * PI / 180.0f)

typedef struct
{
    float value[16];
} mat4;

typedef struct
{
    float x;
    float y;
    float z;
} vec3;

typedef struct
{
    float x;
    float y;
} vec2;

static vec3 AXIS_UP = {0.0f, 1.0f, 0.0f};
static vec3 AXIS_RIGHT = {1.0f, 0.0f, 0.0f};
static vec3 AXIS_FRONT = {0.0f, 0.0f, 1.0f};

static mat4 TEMP_MAT;

void normalize(vec3 *v);

void identity(mat4 *m);
void translate(mat4 *m, vec3 *v);
void translate_v2(mat4 *m, vec2 *v);
void rotate(mat4 *m, vec3 *axis, float angle);

void add_v3(vec3 *v, vec3 *v1, vec3 *v2);
void add_v2(vec2 *v, vec2 *v1, vec2 *v2);

void subtract_v2(vec2 *v, vec2 *v1, vec2 *v2);

void multiply(mat4 *m, mat4 *m1, mat4 *m2);
void multiply_v3f(vec3 *v, vec3 *v1, float v2);
void multiply_v2f(vec2 *v, vec2 *v1, float v2);

void frustum(mat4 *m, float left, float right, float bottom, float top, float znear, float zfar);
void perspective(mat4 *m, float fov, float aspect, float znear, float zfar);
void ortho(mat4 *m, float left, float right, float bottom, float top, float near, float far);

#endif
