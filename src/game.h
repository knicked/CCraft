#ifndef GAME_H
#define GAME_H

#include "world.h"
#include "gui.h"
#include "miniz.h"

typedef struct
{
    GLFWwindow *window;
    float window_width;
    float window_height;

    world w;
    gui gui;

    gui_text *debug_text;

    int online;
    char *buffer;
    z_stream inf_stream;
    struct sockaddr_in server_addr;
    SOCKET server_socket;
    char player_nickname[31];
    fd_set read_fds;
    struct timeval tv;
} game;

void game_init(game *g, GLFWwindow *window);
void game_destroy(game *g);
void game_handle_input(game *g, input *i);
void game_tick(game *g);
void game_draw(game *g, double delta_time, double time_since_tick);

#endif
