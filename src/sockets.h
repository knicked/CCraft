#ifndef SOCKETS_H
#define SOCKETS_H

#ifdef _WIN32
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
    #endif
    #include <winsock2.h>
    #include <Ws2tcpip.h>
    #define SOCKET_VALID(s) ((s) != INVALID_SOCKET)
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #define SOCKET_VALID(s) ((s) != -1)
    typedef int SOCKET;
#endif

#include "block.h"

#define MAX_PLAYERS 32
#define DATA_BUFFER_SIZE 32768

#define SET_BLOCK_ID 0
#define SPAWN_PLAYER_ID 1
#define DESPAWN_PLAYER_ID 2
#define POSITION_UPDATE_ID 3
#define CHUNK_DATA_ID 4

typedef struct
{
    unsigned char id;
    block_id block;
    short x;
    short y;
    short z;
} set_block_packet;

typedef struct
{
    unsigned char id;
    char x;
    char z;
    unsigned short length;
    unsigned short complete;
    char data[1024];
} chunk_data_packet;

typedef struct
{
    unsigned char id;
    unsigned char player_id;
} spawn_player_packet;

typedef struct
{
    unsigned char id;
    unsigned char player_id;
} despawn_player_packet;

typedef struct
{
    unsigned char id;
    unsigned char player_id;
    short x;
    short y;
    short z;
} position_update_packet;

#endif
