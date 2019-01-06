#include "game.h"

#include <stdio.h>
#include <math.h>

void framebuffer_size_callback(GLFWwindow *window, int x, int y)
{
    glViewport(0, 0, x, y);
    game *g = (game *) glfwGetWindowUserPointer(window);
    g->window_width = x;
    g->window_height = y;
    game_draw(g);
    glfwSwapBuffers(window);
}

void cursor_pos_callback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    game *g = (game *) glfwGetWindowUserPointer(window);
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    g->mouse_pos = (vec2){(float) mouse_x, (float) mouse_y};
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    game *g = (game *) glfwGetWindowUserPointer(window);
    g->mouse_buttons[button] = action == GLFW_PRESS ? 1 : 0;
    if (!g->mouse_locked && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, g->mouse_pos.x, g->mouse_pos.y);
        g->mouse_locked = 1;
    }
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset)
{
    game *g = (game *) glfwGetWindowUserPointer(window);
    g->selected_block += y_offset;
    if (g->selected_block < 1) g->selected_block = 1;
    else if (g->selected_block > 255) g->selected_block = 255;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    game *g = (game *) glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_F)
        {
            g->print_fps = !g->print_fps;
        }
        else if (key == GLFW_KEY_ESCAPE)
        {
            if (g->mouse_locked)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                g->mouse_locked = 0;
            }
        }
        else if (key == GLFW_KEY_V)
        {
            g->v_sync = !g->v_sync;
            glfwSwapInterval(g->v_sync);
        }
    }
}

void game_init(game *g, GLFWwindow *window)
{
    g->window = window;
    g->window_width = 1280.0f;
    g->window_height = 720.0f;
    g->sensitivity = 0.1f;
    g->print_fps = 0;
    glfwSwapInterval(1);
    g->v_sync = 1;

    glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);
    glfwSetScrollCallback(window, &scroll_callback);
    glfwSetKeyCallback(window, &key_callback);
    glfwSetCursorPosCallback(window, &cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    world_init(&g->w);

    g->selected_block = COBBLESTONE;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    g->mouse_locked = 1;
    double mouse_x, mouse_y;
    glfwGetCursorPos(g->window, &mouse_x, &mouse_y);
    g->last_mouse_pos = (vec2){(float) mouse_x, (float) mouse_y};
    
    g->mouse_buttons[0] = 0;
    g->mouse_buttons[1] = 0;
    g->mouse_buttons[2] = 0;
}

void game_destroy(game *g)
{
    world_destroy(&g->w);

    glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void game_draw(game *g)
{
    g->cur_time = glfwGetTime();
    g->delta_time = g->cur_time - g->last_time;
    g->last_time = g->cur_time;

    if (g->print_fps)
    printf("FPS: %d\n", (int) roundf(1.0f / g->delta_time));

    subtract_v2(&g->mouse_delta, &g->last_mouse_pos, &g->mouse_pos);
    g->last_mouse_pos = g->mouse_pos;

    vec3 move_dir = {0.0f, 0.0f, 0.0f};

    if (glfwGetKey(g->window, GLFW_KEY_SPACE) == GLFW_PRESS)
        move_dir.y += 1.0f;
    if (glfwGetKey(g->window, GLFW_KEY_W) == GLFW_PRESS)
    {
        move_dir.x -= sinf(RADIANS(g->w.camera_rotation.y));
        move_dir.z -= cosf(RADIANS(g->w.camera_rotation.y));
    }
    if (glfwGetKey(g->window, GLFW_KEY_A) == GLFW_PRESS)
    {
        move_dir.x -= cosf(RADIANS(g->w.camera_rotation.y));
        move_dir.z += sinf(RADIANS(g->w.camera_rotation.y));
    }
    if (glfwGetKey(g->window, GLFW_KEY_S) == GLFW_PRESS)
    {
        move_dir.x += sinf(RADIANS(g->w.camera_rotation.y));
        move_dir.z += cosf(RADIANS(g->w.camera_rotation.y));
    }
    if (glfwGetKey(g->window, GLFW_KEY_D) == GLFW_PRESS)
    {
        move_dir.x += cosf(RADIANS(g->w.camera_rotation.y));
        move_dir.z -= sinf(RADIANS(g->w.camera_rotation.y));
    }
    if (glfwGetKey(g->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        move_dir.y -= 1.0f;

    vec2 rotation_amount = {0.0f, 0.0f};

    if (glfwGetKey(g->window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rotation_amount.y += 90.0f;
    if (glfwGetKey(g->window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rotation_amount.y -= 90.0f;
    if (glfwGetKey(g->window, GLFW_KEY_UP) == GLFW_PRESS)
        rotation_amount.x += 90.0f;
    if (glfwGetKey(g->window, GLFW_KEY_DOWN) == GLFW_PRESS)
        rotation_amount.x -= 90.0f;

    multiply_v2f(&rotation_amount, &rotation_amount, g->delta_time);
    rotation_amount.x += g->mouse_delta.y * g->sensitivity;
    rotation_amount.y += g->mouse_delta.x * g->sensitivity;
    add_v2(&g->w.camera_rotation, &g->w.camera_rotation, &rotation_amount);

    if (move_dir.x != 0.0f || move_dir.y != 0.0f || move_dir.z != 0.0f)
    {
        normalize(&move_dir);
        vec3 move_amount;
        multiply_v3f(&move_amount, &move_dir, g->delta_time * 10.0f);
        add_v3(&g->w.camera_position, &g->w.camera_position, &move_amount);
    }

    if (g->mouse_locked)
    {
        if (g->mouse_buttons[0])
            world_set_block(&g->w, roundf(g->w.camera_position.x), roundf(g->w.camera_position.y) - 1.6f, roundf(g->w.camera_position.z), AIR);
        if (g->mouse_buttons[1])
            world_set_block(&g->w, roundf(g->w.camera_position.x), roundf(g->w.camera_position.y) - 1.6f, roundf(g->w.camera_position.z), g->selected_block);
    }

    g->w.window_width = g->window_width;
    g->w.window_height = g->window_height;

    world_draw(&g->w);
}