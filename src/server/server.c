#include "server.h"

#include <stdio.h>
#include <signal.h>

void server_init(server *s, unsigned short port)
{
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(1,1), &wsa_data);
    #endif

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
                    printf("New connection!\n");
                    spawn_player_packet packet;
                    packet.id = SPAWN_PLAYER_ID;
                    packet.player_id = new_fd;
                    for (int j = 0; j <= s->max_fd; j++)
                    {
                        if (j != s->listener && j != new_fd)
                        {
                            send(j, &packet, sizeof(packet), 0);
                        }
                    }
                    for (int j = 0; j < s->num_players; j++)
                    {
                        packet.id = SPAWN_PLAYER_ID;
                        packet.player_id = s->players[j].id;
                        send(new_fd, &packet, sizeof(packet), 0);
                    }
                    s->players[s->num_players].id = new_fd;
                    s->num_players++;

                }
                else
                {
                    int data_size = 0;
                    int data_position = 0;
                    if (SOCKET_VALID(data_size = recv(i, s->buffer, sizeof(s->buffer), 0)))
                    {
                        if (data_size == 0)
                        {
                            printf("Client disconnected.\n");
                            close(i);
                            FD_CLR(i, &s->sockets);
                            int index;
                            for (int j = 0; j < s->num_players; j++)
                            {
                                if (s->players[j].id == i)
                                {
                                    index = j;
                                    break;
                                }
                            }
                            for (int j = index; j < s->num_players - 1; j++)
                            {
                                s->players[j] = s->players[j + 1];
                            }
                            s->num_players--;
                            despawn_player_packet packet;
                            packet.id = DESPAWN_PLAYER_ID;
                            packet.player_id = i;
                            for (int j = 0; j <= s->max_fd; j++)
                            {
                                if (j != s->listener && j != i)
                                {
                                    send(j, &packet, sizeof(packet), 0);
                                }
                            }
                        }
                        else
                        {
                            while (data_position < data_size)
                            {
                                switch (s->buffer[data_position])
                                {
                                    case SET_BLOCK_ID:
                                    {
                                        set_block_packet *packet = (set_block_packet *) (s->buffer + data_position);
                                        for (int j = 0; j <= s->max_fd; j++)
                                        {
                                            if (j != s->listener && j != i)
                                            {
                                                send(j, packet, sizeof(set_block_packet), 0);
                                            }
                                        }
                                        printf("The block at %d/%d/%d has been set to %d\n", (short) ntohs(packet->x), (short) ntohs(packet->y), (short) ntohs(packet->z), packet->block);
                                        data_position += sizeof(set_block_packet);
                                    }
                                    break;
                                    case POSITION_UPDATE_ID:
                                    {
                                        position_update_packet *packet = (position_update_packet *) (s->buffer + data_position);
                                        for (int j = 0; j < s->num_players; j++)
                                        {
                                            if (s->players[j].id == i)
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
            for (int j = 0; j < s->num_players; j++)
            {
                if (s->players[j].id == i)
                {
                    position_update_packet packet;
                    packet.id = POSITION_UPDATE_ID;
                    packet.player_id = s->players[j].id;
                    packet.x = htons(s->players[j].x);
                    packet.y = htons(s->players[j].y);
                    packet.z = htons(s->players[j].z);
                    for (int k = 0; k <= s->max_fd; k++)
                    {
                        if (k != s->listener && k != i)
                        {
                            send(k, &packet, sizeof(packet), 0);
                        }
                    }
                    break;
                }
            }
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
}
