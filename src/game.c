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
        #ifdef _WIN32
            closesocket(g->server_socket);
        #else
            close(g->server_socket);
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
            int data_size = 0;
            int data_position = 0;
            if (SOCKET_VALID(data_size = recv(g->server_socket, g->buffer, sizeof(g->buffer), 0)))
            {
                if (data_size == 0)
                {
                    printf("Disconnected from the server.\n");
                    g->online = 0;
                    #ifdef _WIN32
                        closesocket(g->server_socket);
                        WSACleanup();
                    #else
                        close(g->server_socket);
                    #endif
                }
                else
                {
                    while (data_position < data_size)
                    {
                        switch (g->buffer[data_position])
                        {
                            case SET_BLOCK_ID:
                            {
                                set_block_packet *packet = (set_block_packet *) (g->buffer + data_position);
                                packet->x = ntohs(packet->x);
                                packet->y = ntohs(packet->y);
                                packet->z = ntohs(packet->z);
                                world_set_block(&g->w, packet->x, packet->y, packet->z, packet->block);
                                data_position += sizeof(set_block_packet);
                            }
                            break;
                            case SPAWN_PLAYER_ID:
                            {
                                spawn_player_packet *packet = (spawn_player_packet *) (g->buffer + data_position);
                                g->w.players[g->w.num_players].id = packet->player_id;
                                g->w.num_players++;
                                printf("Player %d joined the game.\n", packet->player_id);
                                data_position += sizeof(spawn_player_packet);
                            }
                            break;
                            case DESPAWN_PLAYER_ID:
                            {
                                despawn_player_packet *packet = (despawn_player_packet *) (g->buffer + data_position);
                                int index = 0;
                                for (int i = 0; i < MAX_PLAYERS - 1; i++)
                                {
                                    if (g->w.players[i].id == packet->player_id)
                                    {
                                        index = i;
                                        break;
                                    }
                                }
                                for (int i = index; i < MAX_PLAYERS - 2; i++)
                                {
                                    g->w.players[i] = g->w.players[i + 1];
                                }
                                g->w.num_players--;
                                printf("Player %d left the game.\n", packet->player_id);
                                data_position += sizeof(despawn_player_packet);
                            }
                            break;
                            case POSITION_UPDATE_ID:
                            {
                                position_update_packet *packet = (position_update_packet *) (g->buffer + data_position);
                                for (int i = 0; i < g->w.num_players; i++)
                                {
                                    if (g->w.players[i].id == packet->player_id)
                                    {
                                        g->w.players[i].x = ntohs(packet->x);
                                        g->w.players[i].y = ntohs(packet->y);
                                        g->w.players[i].z = ntohs(packet->z);
                                        break;
                                    }
                                }
                                data_position += sizeof(position_update_packet);
                            }
                            break;
                        }
                    }
                }
            }
        }

        if (g->w.block_changed)
        {
            set_block_packet packet;
            packet.id = SET_BLOCK_ID;
            packet.x = htons((short) g->w.selected_block_x);
            packet.y = htons((short) g->w.selected_block_y);
            packet.z = htons((short) g->w.selected_block_z);
            packet.block = g->w.new_block;

            send(g->server_socket, &packet, sizeof(packet), 0);
        }

        position_update_packet packet;
        packet.id = POSITION_UPDATE_ID;
        packet.player_id = 255;
        packet.x = htons(g->w.player.position.x * 32.0f);
        packet.y = htons(g->w.player.position.y * 32.0f);
        packet.z = htons(g->w.player.position.z * 32.0f);
        send(g->server_socket, &packet, sizeof(packet), 0);
    }
    else
    {
        g->w.num_players = 0;
    }
}

void game_draw(game *g, double delta_time)
{
    glViewport(0, 0, g->window_width, g->window_height);

    world_draw(&g->w, delta_time);
    gui_draw(&g->gui);
}
