#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>

void server_init(server *s, unsigned short port)
{
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(1,1), &wsa_data);
    #endif

    s->buffer = malloc(DATA_BUFFER_SIZE);
    s->chunks = malloc(WORLD_SIZE * WORLD_SIZE * sizeof(chunk));
    memset(s->chunks, AIR, WORLD_SIZE * WORLD_SIZE * sizeof(chunk));
    memset(s->ids_used, 0, sizeof(s->ids_used));

    static const int GRASS_LEVEL = 40;

    for (int chunk_x = 0; chunk_x < WORLD_SIZE; chunk_x++)
    {
        for (int chunk_z = 0; chunk_z < WORLD_SIZE; chunk_z++)
        {
            chunk *c = &s->chunks[chunk_x * WORLD_SIZE + chunk_z];

            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                for (int y = 0; y < WORLD_HEIGHT; y++)
                {
                    for (int z = 0; z < CHUNK_SIZE; z++)
                    {
                        if (y > GRASS_LEVEL)
                            c->blocks[x][y][z] = AIR;
                        else if (y == GRASS_LEVEL)
                            c->blocks[x][y][z] = GRASS;
                        else if (y == 0)
                            c->blocks[x][y][z] = BEDROCK;
                        else if (y < 25)
                            c->blocks[x][y][z] = STONE;
                        else if (y < GRASS_LEVEL)
                            c->blocks[x][y][z] = DIRT;
                    }
                }
            }
        }
    }

    FD_ZERO(&s->sockets);

    struct sockaddr_in addrport;
    s->listener = socket(PF_INET, SOCK_STREAM, 0);
    addrport.sin_family = AF_INET;
    addrport.sin_port = htons(port);
    addrport.sin_addr.s_addr = htonl(INADDR_ANY);

    if (SOCKET_VALID(bind(s->listener, (struct sockaddr *) &addrport, sizeof(addrport))))
    {
        if (SOCKET_VALID(listen(s->listener, 10)))
        {
            printf("Server listening for connections on port %d\n", port);
            FD_SET(s->listener, &s->sockets);
            s->max_fd = s->listener;
            #ifndef _WIN32
                signal(SIGPIPE, SIG_IGN);
            #endif
        }
        else
            printf("Server socket can't listen for connections.\n");
    }
    else
    {
        printf("Couldn't bind the socket.\n");
    }

    s->num_players = 0;
}

int find_player_by_socket(server *s, SOCKET socket)
{
    int index = -1;
    for (int i = 0; i < s->num_players; i++)
    {
        if (s->players[i].socket == socket)
        {
            index = i;
            break;
        }
    }
    return index;
}


void spawn_player(server *s, SOCKET player_socket, const char *nickname)
{
    player *new_player = &s->players[s->num_players];
    memset(new_player, 0, sizeof(player));

    new_player->socket = player_socket;
    strcpy(new_player->nickname, nickname);

    for (unsigned char i = 0; i < 255; i++)
    {
        if (!s->ids_used[i])
        {
            new_player->id = i;
            s->ids_used[i] = 1;
            break;
        }
    }

    spawn_player_packet packet;
    packet.id = SPAWN_PLAYER_ID;
    packet.player_id = new_player->id;
    strcpy(packet.nickname, new_player->nickname);
    for (int i = 0; i < s->num_players; i++)
    {
        send(s->players[i].socket, &packet, sizeof(packet), 0);
    }

    for (int i = 0; i < s->num_players; i++)
    {
        packet.player_id = s->players[i].id;
        strcpy(packet.nickname, s->players[i].nickname);
        send(player_socket, &packet, sizeof(packet), 0);
    }

    printf("%s joined the game.\n", new_player->nickname);

    s->num_players++;
}

void despawn_player(server *s, int player_index)
{
    player *p = &s->players[player_index];

    printf("%s left the game.\n", p->nickname);
    close(p->socket);
    FD_CLR(p->socket, &s->sockets);

    despawn_player_packet packet;
    packet.id = DESPAWN_PLAYER_ID;
    packet.player_id = p->id;

    for (int i = player_index; i < s->num_players - 1; i++)
    {
        s->players[i] = s->players[i + 1];
    }
    s->num_players--;

    for (int i = 0; i < s->num_players; i++)
    {
        send(s->players[i].socket, &packet, sizeof(packet), 0);
    }

    s->ids_used[p->id] = 0;
}

void server_tick(server *s)
{
    fd_set read_fds = s->sockets;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if (SOCKET_VALID(select(s->max_fd + 1, &read_fds, NULL, NULL, &tv)))
    {
        for (int i = 0; i <= s->max_fd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == s->listener)
                {
                    struct sockaddr_storage client_addr;
                    socklen_t addr_len = sizeof(client_addr);
                    SOCKET new_fd = accept(s->listener, (struct sockaddr *) &client_addr, &addr_len);
                    FD_SET(new_fd, &s->sockets);
                    if (new_fd > s->max_fd) s->max_fd = new_fd;
                }
                else
                {
                    int data_size = 0;
                    int data_position = 0;
                    if (SOCKET_VALID(data_size = recv(i, s->buffer, DATA_BUFFER_SIZE, 0)))
                    {
                        if (data_size == 0)
                        {
                            int index = find_player_by_socket(s, i);
                            if (index != -1)
                            {
                                despawn_player(s, index);
                            }
                        }
                        else
                        {
                            while (data_position < data_size)
                            {
                                switch (s->buffer[data_position])
                                {
                                    case PLAYER_IDENTIFICATION_ID:
                                    {
                                        player_identification_packet *packet = (player_identification_packet *) (s->buffer + data_position);
                                        if (find_player_by_socket(s, i) == -1)
                                        {
                                            packet->nickname[30] = '\0';
                                            spawn_player(s, i, packet->nickname);
                                        }
                                        data_position += sizeof(player_identification_packet);
                                    }
                                    break;
                                    case SET_BLOCK_ID:
                                    {
                                        set_block_packet *packet = (set_block_packet *) (s->buffer + data_position);

                                        short x = ntohs(packet->x);
                                        short y = ntohs(packet->y);
                                        short z = ntohs(packet->z);

                                        if (y >= WORLD_HEIGHT || y < 0 ||
                                            x >= CHUNK_SIZE * WORLD_SIZE / 2 || x <= -CHUNK_SIZE * WORLD_SIZE / 2 ||
                                            z >= CHUNK_SIZE * WORLD_SIZE / 2 || z <= -CHUNK_SIZE * WORLD_SIZE / 2)
                                        {
                                            int index = find_player_by_socket(s, i);
                                            if (index != -1)
                                            {
                                                printf("%s has tried to set a block at invalid coordinates, kicking him...\n", s->players[index].nickname);
                                                despawn_player(s, index);
                                            }
                                        }
                                        else
                                        {
                                            for (int j = 0; j <= s->max_fd; j++)
                                            {
                                                if (j != s->listener && j != i)
                                                {
                                                    send(j, packet, sizeof(set_block_packet), 0);
                                                }
                                            }

                                            size_t chunk_x = CHUNK_FROM_WORLD_COORDS(x);
                                            size_t chunk_z = CHUNK_FROM_WORLD_COORDS(z);
                                            chunk *c = &s->chunks[chunk_x * WORLD_SIZE + chunk_z];

                                            size_t block_x = WORLD_TO_CHUNK(x);
                                            size_t block_z = WORLD_TO_CHUNK(z);
                                            c->blocks[block_x][y][block_z] = packet->block;

                                            printf("The block at %d/%d/%d has been set to %d\n", x, y, z, packet->block);
                                        }
                                        data_position += sizeof(set_block_packet);
                                    }
                                    break;
                                    case POSITION_UPDATE_ID:
                                    {
                                        position_update_packet *packet = (position_update_packet *) (s->buffer + data_position);
                                        for (int j = 0; j < s->num_players; j++)
                                        {
                                            if (s->players[j].socket == i)
                                            {
                                                s->players[j].x = ntohs(packet->x);
                                                s->players[j].y = ntohs(packet->y);
                                                s->players[j].z = ntohs(packet->z);
                                                break;
                                            }
                                        }
                                        data_position += sizeof(position_update_packet);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0; i < s->num_players; i++)
        {
            player *p = &s->players[i];
            int bytes_to_send = 0;
            for (int j = 0; j < s->num_players; j++)
            {
                if (j != i)
                {
                    position_update_packet *packet = (position_update_packet *) (s->buffer + bytes_to_send);
                    packet->id = POSITION_UPDATE_ID;
                    packet->player_id = s->players[j].id;
                    packet->x = htons(s->players[j].x);
                    packet->y = htons(s->players[j].y);
                    packet->z = htons(s->players[j].z);
                    bytes_to_send += sizeof(position_update_packet);
                }
            }
            chunk *chunk_to_send = NULL;
            size_t chunk_to_send_x;
            size_t chunk_to_send_z;
            unsigned short chunk_to_send_complete;
            float min_distance = -1.0f;

            for (int x = 0; x < WORLD_SIZE; x++)
            {
                for (int z = 0; z < WORLD_SIZE; z++)
                {
                    if (p->chunk_data_sent[x][z] < CHUNK_SIZE * WORLD_HEIGHT * CHUNK_SIZE)
                    {
                        chunk *c = &s->chunks[x * WORLD_SIZE + z];

                        float distance_x = abs(x - CHUNK_FROM_WORLD_COORDS((int) floorf(s->players[i].x / 32.0f)));
                        float distance_z = abs(z - CHUNK_FROM_WORLD_COORDS((int) floorf(s->players[i].z / 32.0f)));
                        float distance = sqrtf(distance_x * distance_x + distance_z * distance_z);
                        if (distance <= 12.0f && (distance < min_distance || min_distance < 0.0f))
                        {
                            min_distance = distance;
                            chunk_to_send = c;
                            chunk_to_send_x = x;
                            chunk_to_send_z = z;
                            chunk_to_send_complete = p->chunk_data_sent[x][z];
                        }
                    }
                }
            }

            if (chunk_to_send)
            {
                chunk_data_packet *packet = (chunk_data_packet *) (s->buffer + bytes_to_send);
                packet->id = CHUNK_DATA_ID;
                packet->x = chunk_to_send_x - WORLD_SIZE / 2;
                packet->z = chunk_to_send_z - WORLD_SIZE / 2;

                s->def_stream.zalloc = Z_NULL;
                s->def_stream.zfree = Z_NULL;
                s->def_stream.opaque = Z_NULL;
                deflateInit(&s->def_stream, 3);

                s->def_stream.avail_in = sizeof(chunk_to_send->blocks) - chunk_to_send_complete;
                s->def_stream.next_in = (Bytef *) ((char *) chunk_to_send->blocks + chunk_to_send_complete);
                s->def_stream.avail_out = sizeof(packet->data);
                s->def_stream.next_out = (Bytef *) packet->data;

                deflate(&s->def_stream, Z_FINISH);
                deflateEnd(&s->def_stream);

                packet->length = htons(s->def_stream.total_in);
                packet->complete = htons(chunk_to_send_complete);

                s->players[i].chunk_data_sent[chunk_to_send_x][chunk_to_send_z] += s->def_stream.total_in;
                bytes_to_send += sizeof(chunk_data_packet);
            }
            send(s->players[i].socket, s->buffer, bytes_to_send, 0);
        }
    }
}

void server_destroy(server *s)
{
    #ifdef _WIN32
        closesocket(s->listener);
        WSACleanup();
    #else
        close(s->listener);
    #endif
    free(s->buffer);
    free(s->chunks);
}
