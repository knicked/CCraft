#include "game.h"

#include <stdio.h>
#include <math.h>

void game_init(game *g, GLFWwindow *window)
{
    g->window = window;
    g->window_width = 1280.0f;
    g->window_height = 720.0f;

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    world_init(&g->w);
    gui_init(&g->gui);
}

void game_destroy(game *g)
{
    world_destroy(&g->w);
    gui_destroy(&g->gui);
}

void game_handle_input(game *g, input *i)
{
    g->window_width = i->window_width;
    g->window_height = i->window_height;

    if (i->keys_down[GLFW_KEY_ESCAPE])
    {
        if (i->mouse_locked)
        {
            input_unlock_mouse(i, g->window);
        }
    }

    if (i->mouse_buttons_down[GLFW_MOUSE_BUTTON_LEFT])
    {
        if (!i->mouse_locked)
        {
            input_lock_mouse(i, g->window);
        }
    }

    world_handle_input(&g->w, i);
    gui_handle_input(&g->gui, i);
}

void game_tick(game *g)
{
    world_tick(&g->w);
}

void game_draw(game *g, double delta_time)
{
    glViewport(0, 0, g->window_width, g->window_height);

    world_draw(&g->w, delta_time);
    gui_draw(&g->gui);
}
