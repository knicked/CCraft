#include "input.h"

#include "game.h"

void framebuffer_size_callback(GLFWwindow *window, int x, int y)
{
    input *i = (input *) glfwGetWindowUserPointer(window);
    i->window_width = x;
    i->window_height = y;
}

void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    input *i = (input *) glfwGetWindowUserPointer(window);
    i->mouse_pos = (vec2) {mouse_x, mouse_y};
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    input *i = (input *) glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS)
    {
        i->mouse_buttons_down[button] = 1;
        i->mouse_buttons[button] = 1;
    }
    else if (action == GLFW_RELEASE)
    {
        i->mouse_buttons_up[button] = 1;
        i->mouse_buttons[button] = 0;
    }
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset)
{
    input *i = (input *) glfwGetWindowUserPointer(window);
    i->scroll_delta = y_offset;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    input *i = (input *) glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS)
    {
        i->keys_down[key] = 1;
        i->keys[key] = 1;
    }
    else if (action == GLFW_RELEASE)
    {
        i->keys_up[key] = 1;
        i->keys[key] = 0;
    }
}

void input_init(input *i, GLFWwindow *window)
{
    glfwSetScrollCallback(window, &scroll_callback);
    glfwSetKeyCallback(window, &key_callback);
    glfwSetCursorPosCallback(window, &cursor_pos_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    i->window_width = width;
    i->window_height = height;

    for (int j = 0; j < 512; j++)
    {
        i->keys[j] = 0;
        i->keys_down[j] = 0;
        i->keys_up[j] = 0;
    }

    for (int j = 0; j < 3; j++)
    {
        i->mouse_buttons[j] = 0;
        i->mouse_buttons_down[j] = 0;
        i->mouse_buttons_up[j] = 0;
    }
    i->mouse_locked = 0;

    i->mouse_sensitivity = 0.1f;
}

void input_end_frame(input *i)
{
    subtract_v2(&i->mouse_delta, &i->last_mouse_pos, &i->mouse_pos);
    i->last_mouse_pos = i->mouse_pos;
    i->scroll_delta = 0.0;
    for (int j = 0; j < 512; j++)
    {
        i->keys_down[j] = 0;
        i->keys_up[j] = 0;
    }
    for (int j = 0; j < 3; j++)
    {
        i->mouse_buttons_down[j] = 0;
        i->mouse_buttons_up[j] = 0;
    }
}

void input_lock_mouse(input *i, GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(window, 0.0, 0.0);
    i->last_mouse_pos.x = 0.0f;
    i->last_mouse_pos.y = 0.0f;
    i->mouse_pos.x = 0.0f;
    i->mouse_pos.y = 0.0f;
    i->mouse_delta.x = 0.0f;
    i->mouse_delta.y = 0.0f;
    i->mouse_locked = 1;
    i->mouse_buttons_down[GLFW_MOUSE_BUTTON_LEFT] = 0;
    i->mouse_buttons[GLFW_MOUSE_BUTTON_LEFT] = 0;
}

void input_unlock_mouse(input *i, GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPos(window, i->window_width / 2.0, i->window_height / 2.0);
    i->mouse_locked = 0;
}