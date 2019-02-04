#ifndef SERVER_H
#define SERVER_H

#include "../sockets.h"

typedef struct
{
    fd_set sockets;
    SOCKET listener;
    SOCKET max_fd;

    network_player players[MAX_PLAYERS];
    unsigned int num_players;

    char buffer[256];
} server;

void server_init(server *s, unsigned short port);
void server_tick(server *s);
void server_destroy(server *s);

#endif
