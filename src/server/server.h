#ifndef SERVER_H
#define SERVER_H

#include "../sockets.h"

#include "miniz.h"

#define CHUNK_SIZE 16
#define WORLD_HEIGHT 128
#define WORLD_SIZE 32

#define WORLD_TO_CHUNK(x) (x < 0 ? x % CHUNK_SIZE == 0 ? 0 : CHUNK_SIZE + x % CHUNK_SIZE : x % CHUNK_SIZE)
#define CHUNK_FROM_WORLD_COORDS(x) ((x / CHUNK_SIZE < 0 ? x + 1 : x) / CHUNK_SIZE + WORLD_SIZE / 2 - (x < 0 ? 1 : 0))

typedef struct
{
    block_id blocks[CHUNK_SIZE][WORLD_HEIGHT][CHUNK_SIZE];
} chunk;

typedef struct
{
    short x;
    short y;
    short z;
    unsigned short chunk_data_sent[WORLD_SIZE][WORLD_SIZE];
    unsigned char id;
    char nickname[31];
    SOCKET socket;
} player;

typedef struct
{
    fd_set sockets;
    SOCKET listener;
    SOCKET max_fd;

    chunk *chunks;

    player players[MAX_PLAYERS];
    unsigned int num_players;
    int ids_used[256];

    char *buffer;
    z_stream def_stream;
} server;

void server_init(server *s, unsigned short port);
void server_tick(server *s);
void server_destroy(server *s);

#endif
