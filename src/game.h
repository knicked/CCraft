#ifndef GAME_H
#define GAME_H

#include "mesh.h"
#include "world.h"
#include "input.h"

typedef struct
{
    GLFWwindow *window;
    float window_width;
    float window_height;
    int v_sync;

    int print_fps;

    world w;
} game;

void game_init(game *g, GLFWwindow *window);
void game_destroy(game *g);
void game_handle_input(game *g, input *i);
void game_update(game *g, double delta_time);
void game_draw(game *g);

#endif