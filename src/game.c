#include "game.h"

#include <stdio.h>
#include <math.h>

int calculate_selected_block(world *w, int *x, int *y, int *z, int *face_x, int *face_y, int *face_z, float radius)
{
    vec3 direction =
    {
        .x = -sinf(RADIANS(w->camera_rotation.y)) * cosf(RADIANS(w->camera_rotation.x)),
        .z = -cosf(RADIANS(w->camera_rotation.y)) * cosf(RADIANS(w->camera_rotation.x)),
        .y = sinf(RADIANS(w->camera_rotation.x))
    };

    *x = roundf(w->camera_position.x);
    *z = roundf(w->camera_position.z);
    *y = roundf(w->camera_position.y);

    int step_x = direction.x > 0.0f ? 1 : direction.x < 0.0f ? -1 : 0;
    int step_z = direction.z > 0.0f ? 1 : direction.z < 0.0f ? -1 : 0;
    int step_y = direction.y > 0.0f ? 1 : direction.y < 0.0f ? -1 : 0;

    float t_max_x = (direction.x > 0.0f ? roundf(w->camera_position.x) + 0.5f - w->camera_position.x : roundf(w->camera_position.x) - 0.5f - w->camera_position.x) / direction.x;
    float t_max_z = (direction.z > 0.0f ? roundf(w->camera_position.z) + 0.5f - w->camera_position.z : roundf(w->camera_position.z) - 0.5f - w->camera_position.z) / direction.z;
    float t_max_y = (direction.y > 0.0f ? roundf(w->camera_position.y) + 0.5f - w->camera_position.y : roundf(w->camera_position.y) - 0.5f - w->camera_position.y) / direction.y;

    float t_delta_x = (float) step_x / direction.x;
    float t_delta_z = (float) step_z / direction.z;
    float t_delta_y = (float) step_y / direction.y;

    while (1)
    {
        if (t_max_x < t_max_y)
        {
            if (t_max_x < t_max_z)
            {
                if (t_max_x > radius) return 0;
                *x += step_x;
                t_max_x += t_delta_x;

                *face_x = -step_x;
                *face_y = 0;
                *face_z = 0;
            }
            else
            {
                if (t_max_z > radius) return 0;
                *z += step_z;
                t_max_z += t_delta_z;

                *face_x = 0;
                *face_y = 0;
                *face_z = -step_z;
            }
        }
        else
        {
            if (t_max_y < t_max_z)
            {
                if (t_max_y > radius) return 0;
                *y += step_y;
                t_max_y += t_delta_y;

                *face_x = 0;
                *face_y = -step_y;
                *face_z = 0;
            }
            else
            {
                if (t_max_z > radius) return 0;
                *z += step_z;
                t_max_z += t_delta_z;

                *face_x = 0;
                *face_y = 0;
                *face_z = -step_z;
            }
        }
        if (world_get_block(w, *x, *y, *z) != AIR)
            return 1;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int x, int y)
{
    glViewport(0, 0, x, y);
    game *g = (game *) glfwGetWindowUserPointer(window);
    g->window_width = x;
    g->window_height = y;
    game_draw(g);
    glfwSwapBuffers(window);
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
    input_init(&g->i, g->window);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    world_init(&g->w);

    input_lock_mouse(&g->i, g->window);

    g->selected_block = 1;
}

void game_destroy(game *g)
{
    if (g->i.mouse_locked)
        input_unlock_mouse(&g->i, g->window);

    world_destroy(&g->w);
}

void game_draw(game *g)
{
    g->cur_time = glfwGetTime();
    g->delta_time = g->cur_time - g->last_time;
    g->last_time = g->cur_time;

    if (g->print_fps)
    printf("FPS: %d\n", (int) roundf(1.0f / g->delta_time));

    if (g->i.keys_down[GLFW_KEY_F])
    {
        g->print_fps = !g->print_fps;
    }

    int selected_block_x;
    int selected_block_y;
    int selected_block_z;
    int selected_face_x;
    int selected_face_y;
    int selected_face_z;

    int block_in_range = calculate_selected_block(&g->w,
    &selected_block_x, &selected_block_y, &selected_block_z,
    &selected_face_x, &selected_face_y, &selected_face_z, 5.0f);

    if (g->i.mouse_locked)
    {
        vec3 move_dir = {0.0f, 0.0f, 0.0f};

        if (g->i.keys[GLFW_KEY_SPACE])
            move_dir.y += 1.0f;
        if (g->i.keys[GLFW_KEY_LEFT_SHIFT])
            move_dir.y -= 1.0f;

        if (g->i.keys[GLFW_KEY_W])
        {
            move_dir.x -= sinf(RADIANS(g->w.camera_rotation.y));
            move_dir.z -= cosf(RADIANS(g->w.camera_rotation.y));
        }
        if (g->i.keys[GLFW_KEY_A])
        {
            move_dir.x -= cosf(RADIANS(g->w.camera_rotation.y));
            move_dir.z += sinf(RADIANS(g->w.camera_rotation.y));
        }
        if (g->i.keys[GLFW_KEY_S])
        {
            move_dir.x += sinf(RADIANS(g->w.camera_rotation.y));
            move_dir.z += cosf(RADIANS(g->w.camera_rotation.y));
        }
        if (g->i.keys[GLFW_KEY_D])
        {
            move_dir.x += cosf(RADIANS(g->w.camera_rotation.y));
            move_dir.z -= sinf(RADIANS(g->w.camera_rotation.y));
        }

        if (move_dir.x != 0.0f || move_dir.y != 0.0f || move_dir.z != 0.0f)
        {
            normalize(&move_dir);
            vec3 move_amount;
            multiply_v3f(&move_amount, &move_dir, g->delta_time * 10.0f);
            add_v3(&g->w.camera_position, &g->w.camera_position, &move_amount);
        }

        if (g->i.scroll_delta != 0.0)
        {
            g->selected_block += g->i.scroll_delta;
            printf("Selected block of ID %d\n", g->selected_block);
        }

        if (g->i.mouse_buttons_down[GLFW_MOUSE_BUTTON_LEFT])
        {
            if (block_in_range)
                world_set_block(&g->w, selected_block_x, selected_block_y, selected_block_z, AIR);
        }
        if (g->i.mouse_buttons_down[GLFW_MOUSE_BUTTON_RIGHT])
        {
            if (block_in_range)
                world_set_block(&g->w, selected_block_x + selected_face_x, selected_block_y + selected_face_y, selected_block_z + selected_face_z, g->selected_block);
        }
    }

    if (g->i.keys_down[GLFW_KEY_ESCAPE])
    {
        if (g->i.mouse_locked)
        {
            input_unlock_mouse(&g->i, g->window);
        }
    }

    if (g->i.mouse_buttons_down[GLFW_MOUSE_BUTTON_LEFT])
    {
        if (!g->i.mouse_locked)
        {
            input_lock_mouse(&g->i, g->window);
        }
    }

    if (g->i.keys_down[GLFW_KEY_V])
    {
        g->v_sync = !g->v_sync;
        glfwSwapInterval(g->v_sync);
    }

    vec2 rotation_amount = {0.0f, 0.0f};

    if (g->i.keys[GLFW_KEY_LEFT])
        rotation_amount.y += 90.0f;
    if (g->i.keys[GLFW_KEY_RIGHT])
        rotation_amount.y -= 90.0f;
    if (g->i.keys[GLFW_KEY_UP])
        rotation_amount.x += 90.0f;
    if (g->i.keys[GLFW_KEY_DOWN])
        rotation_amount.x -= 90.0f;

    multiply_v2f(&rotation_amount, &rotation_amount, g->delta_time);
    if (g->i.mouse_locked)
    {
        rotation_amount.x += g->i.mouse_delta.y * g->sensitivity;
        rotation_amount.y += g->i.mouse_delta.x * g->sensitivity;
    }
    add_v2(&g->w.camera_rotation, &g->w.camera_rotation, &rotation_amount);

    g->w.camera_rotation.x = g->w.camera_rotation.x > 89.0f ? 89.0f : g->w.camera_rotation.x;
    g->w.camera_rotation.x = g->w.camera_rotation.x < -89.0f ? -89.0f : g->w.camera_rotation.x;

    g->w.window_width = g->window_width;
    g->w.window_height = g->window_height;

    world_draw(&g->w);

    input_end_frame(&g->i);
}