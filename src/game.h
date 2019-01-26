#ifndef GAME_H
#define GAME_H

#include "mesh.h"
#include "world.h"
#include "input.h"
#include "gui.h"

typedef struct
{
    GLFWwindow *window;
    float window_width;
    float window_height;

    world w;
    gui gui;
} game;

void game_init(game *g, GLFWwindow *window);
void game_destroy(game *g);
void game_handle_input(game *g, input *i);
void game_tick(game *g);
void game_draw(game *g, double delta_time);

#endif
