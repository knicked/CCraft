#ifndef INPUT_H
#define INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glmath.h"

typedef struct
{
    float window_width;
    float window_height;

    int keys[512];
    int keys_down[512];
    int keys_up[512];

    float mouse_sensitivity;

    int mouse_buttons[3];
    int mouse_buttons_down[3];
    int mouse_buttons_up[3];
    int mouse_locked;
    double scroll_delta;

    vec2 mouse_pos;
    vec2 last_mouse_pos;
    vec2 mouse_delta;
} input;

void input_init(input *i, GLFWwindow *window);
void input_end_frame(input *i);

void input_lock_mouse(input *i, GLFWwindow *window);
void input_unlock_mouse(input *i, GLFWwindow *window);

#endif