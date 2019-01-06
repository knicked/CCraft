#ifndef GAME_H
#define GAME_H

#include "mesh.h"
#include "world.h"

typedef struct
{
    GLFWwindow *window;
    float window_width;
    float window_height;
    int v_sync;

    float sensitivity;

    int mouse_buttons[3];
    int mouse_locked;
    vec2 mouse_pos;
    vec2 last_mouse_pos;
    vec2 mouse_delta;

    int print_fps;

    block_id selected_block;

    double delta_time;
    double last_time;
    double cur_time;

    world w;
} game;

void game_init(game *g, GLFWwindow *window);
void game_destroy(game *g);
void game_draw(game *g);

#endif