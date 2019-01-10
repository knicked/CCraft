#include "input.h"

#include "game.h"

void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    input *i = &((game *) glfwGetWindowUserPointer(window))->i;
    i->mouse_pos = (vec2) {mouse_x, mouse_y};
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    input *i = &((game *) glfwGetWindowUserPointer(window))->i;
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
    input *i = &((game *) glfwGetWindowUserPointer(window))->i;
    i->scroll_delta = y_offset;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    input *i = &((game *) glfwGetWindowUserPointer(window))->i;
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
}

void input_unlock_mouse(input *i, GLFWwindow *window)
{
    game *g = (game *) glfwGetWindowUserPointer(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPos(window, g->window_width / 2.0, g->window_height / 2.0);
    i->mouse_locked = 0;
}