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
    gui_init(&g->gui, &g->w);


    if (g->online)
    {
        printf("Connecting to the server...\n");

        #ifdef _WIN32
            WSADATA wsa_data;
            WSAStartup(MAKEWORD(1,1), &wsa_data);
        #endif

        g->server_socket = socket(PF_INET, SOCK_STREAM, 0);
        g->server_addr.sin_family = AF_INET;
        if (SOCKET_VALID(connect(g->server_socket, (struct sockaddr *) &g->server_addr, sizeof(g->server_addr))))
        {
            printf("Successfully connected to the server.\n");
            g->online = 1;
        }
        else
        {
            printf ("Couldn't connect to the server.\n");
            close(g->server_socket);
        }
        g->tv.tv_sec = 0;
        g->tv.tv_usec = 0;
    }
}

void game_destroy(game *g)
{
    world_destroy(&g->w);
    gui_destroy(&g->gui);

    if (g->online)
    {
        close(g->server_socket);

        #ifdef _WIN32
            WSACleanup();
        #endif
    }
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

    if (g->online)
    {
        FD_ZERO(&g->read_fds);
        FD_SET(g->server_socket, &g->read_fds);

        select(g->server_socket + 1, &g->read_fds, NULL, NULL, &g->tv);

        if (FD_ISSET(g->server_socket, &g->read_fds))
        {
            int data_size;
            if (SOCKET_VALID(data_size = recv(g->server_socket, g->buffer, sizeof(g->buffer), 0)))
            {
                if (data_size == 0)
                {
                    printf("Disconnected from the server.\n");
                    close(g->server_socket);
                }
                else
                {
                    if (data_size == sizeof(set_block_packet))
                    {
                        set_block_packet *packet = (set_block_packet *) g->buffer;
                        packet->x = ntohs(packet->x);
                        packet->y = ntohs(packet->y);
                        packet->z = ntohs(packet->z);
                        world_set_block(&g->w, packet->x, packet->y, packet->z, packet->block);
                    }
                }
            }
        }

        if (g->w.block_changed)
        {
            set_block_packet packet;
            packet.x = htons((short) g->w.selected_block_x);
            packet.y = htons((short) g->w.selected_block_y);
            packet.z = htons((short) g->w.selected_block_z);
            packet.block = g->w.new_block;

            send(g->server_socket, &packet, sizeof(packet), 0);
        }
    }
}

void game_draw(game *g, double delta_time)
{
    glViewport(0, 0, g->window_width, g->window_height);

    world_draw(&g->w, delta_time);
    gui_draw(&g->gui);
}
