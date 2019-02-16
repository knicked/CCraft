#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int recv_all(SOCKET s, char *buf, size_t buf_size)
{
    int data_size = recv(s, buf, buf_size, 0);
    int data_position = 0;

    if (data_size == 0)
        return 0;

    while (data_position < data_size)
    {
        switch (buf[data_position])
        {
            case SET_BLOCK_ID:
            {
                data_position += sizeof(set_block_packet);
            }
            break;
            case SPAWN_PLAYER_ID:
            {
                data_position += sizeof(spawn_player_packet);
            }
            break;
            case DESPAWN_PLAYER_ID:
            {
                data_position += sizeof(despawn_player_packet);
            }
            break;
            case POSITION_UPDATE_ID:
            {
                data_position += sizeof(position_update_packet);
            }
            break;
            case CHUNK_DATA_ID:
            {
                data_position += sizeof(chunk_data_packet);
            }
            break;
        }

        if (data_size < data_position)
        {
            data_size += recv(s, buf + data_size, data_position - data_size, 0);
        }
    }

    return data_size;
}

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

    world_init(&g->w);
    gui_init(&g->gui, &g->w);

    g->debug_text = gui_create_text(&g->gui);

    if (g->online)
    {
        printf("Connecting to the server...\n");
        g->server_socket = socket(PF_INET, SOCK_STREAM, 0);
        g->server_addr.sin_family = AF_INET;
        if (SOCKET_VALID(connect(g->server_socket, (struct sockaddr *) &g->server_addr, sizeof(g->server_addr))))
        {
            printf("Successfully connected to the server.\n");
            g->buffer = malloc(DATA_BUFFER_SIZE);
        }
        else
        {
            printf ("Couldn't connect to the server.\n");
            g->online = 0;
            close(g->server_socket);
        }
        g->tv.tv_sec = 0;
        g->tv.tv_usec = 0;
    }

    if (!g->online) world_generate(&g->w);
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
        free(g->buffer);
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
        for (int i = 0; i < g->w.num_players; i++)
        {
            g->w.players[i].prev_position = g->w.players[i].position;
        }

        FD_ZERO(&g->read_fds);
        FD_SET(g->server_socket, &g->read_fds);

        select(g->server_socket + 1, &g->read_fds, NULL, NULL, &g->tv);

        if (FD_ISSET(g->server_socket, &g->read_fds))
        {
            int data_size = 0;
            int data_position = 0;
            if (SOCKET_VALID(data_size = recv_all(g->server_socket, g->buffer, DATA_BUFFER_SIZE)))
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
                                        g->w.players[i].position = (vec3)
                                        {
                                            (short) ntohs(packet->x) / 32.0f,
                                            (short) ntohs(packet->y) / 32.0f + g->w.player.box.size.y * 0.5f,
                                            (short) ntohs(packet->z) / 32.0f
                                        };
                                        break;
                                    }
                                }
                                data_position += sizeof(position_update_packet);
                            }
                            break;
                            case CHUNK_DATA_ID:
                            {
                                chunk_data_packet *packet = (chunk_data_packet *) (g->buffer + data_position);
                                packet->complete = ntohs(packet->complete);
                                packet->length = ntohs(packet->length);

                                size_t chunk_x = packet->x + WORLD_SIZE / 2;
                                size_t chunk_z = packet->z + WORLD_SIZE / 2;
                                chunk *c = &g->w.chunks[chunk_x * WORLD_SIZE + chunk_z];

                                g->inf_stream.zalloc = Z_NULL;
                                g->inf_stream.zfree = Z_NULL;
                                g->inf_stream.opaque = Z_NULL;

                                g->inf_stream.avail_in = sizeof(packet->data);
                                g->inf_stream.next_in = packet->data;
                                g->inf_stream.avail_out = sizeof(c->blocks) - packet->complete;
                                g->inf_stream.next_out = (char *) c->blocks + packet->complete;

                                inflateInit(&g->inf_stream);
                                inflate(&g->inf_stream, Z_NO_FLUSH);
                                inflateEnd(&g->inf_stream);

                                if (packet->complete + packet->length == CHUNK_SIZE * WORLD_HEIGHT * CHUNK_SIZE)
                                {
                                    c->dirty = 1;
                                    g->w.chunks[(chunk_x == 0 ? chunk_x : chunk_x - 1) * WORLD_SIZE + chunk_z].dirty = 1;
                                    g->w.chunks[(chunk_x == WORLD_SIZE - 1 ? chunk_x : chunk_x + 1) * WORLD_SIZE + chunk_z].dirty = 1;
                                    g->w.chunks[chunk_x * WORLD_SIZE + (chunk_z == 0 ? chunk_z : chunk_z - 1)].dirty = 1;
                                    g->w.chunks[chunk_x * WORLD_SIZE + (chunk_z == WORLD_SIZE - 1 ? chunk_z : chunk_z + 1)].dirty = 1;
                                }

                                data_position += sizeof(chunk_data_packet);
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

void game_draw(game *g, double delta_time, double time_since_tick)
{
    glViewport(0, 0, g->window_width, g->window_height);

    char text[128];
    sprintf(text, "CCraft 0.0.0\n%d fps\n", (int) roundf(1.0f / delta_time));
    if (g->w.noclip_mode)
        strcat(text, "Noclip mode\n");
    else if (g->w.fly_mode)
        strcat(text, "Fly mode\n");
    gui_set_text(g->debug_text, text, 8.0f);
    g->debug_text->position = (vec2) {
        -g->window_width / 2.0f / g->gui.scale + 2.0f,
        g->window_height / 2.0f / g->gui.scale - 10.0f,
    };

    world_draw(&g->w, delta_time, time_since_tick);
    gui_draw(&g->gui);
}
