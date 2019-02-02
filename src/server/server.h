#ifndef SERVER_H
#define SERVER_H

#include "../sockets.h"

typedef struct
{
    fd_set sockets;
    SOCKET listener;
    SOCKET max_fd;

    char buffer[256];
} server;

void server_init(server *s, unsigned short port);
void server_tick(server *s);
void server_destroy(server *s);

#endif
