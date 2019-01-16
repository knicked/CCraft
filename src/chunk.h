#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "mesh.h"

#include <glad/glad.h>

#define CHUNK_SIZE 16
#define WORLD_HEIGHT 256

typedef struct
{
    block_id blocks[CHUNK_SIZE][WORLD_HEIGHT][CHUNK_SIZE];
    int x;
    int z;
    int dirty;
    GLuint vao;
    GLuint buffer;
    GLuint vert_count;
} chunk;

void chunk_build_buffer(chunk *c, void *w, block_vertex *data_buffer);
void chunk_init(chunk *c, int x, int z, GLuint position_location, GLuint normal_location, GLuint tex_coord_location);
void chunk_destroy(chunk *c);

#endif