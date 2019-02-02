#ifndef SOCKETS_H
#define SOCKETS_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <Ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #define SOCKET_VALID(s) ((s) != -1)
    typedef int SOCKET;
#endif

#include "block.h"

typedef struct
{
    short x;
    short y;
    short z;
    block_id block;
} set_block_packet;

#endif
