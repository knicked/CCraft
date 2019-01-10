#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"

#define WORLD_SIZE 32

#define WORLD_TO_CHUNK(x) (x < 0 ? x % CHUNK_SIZE == 0 ? 0 : CHUNK_SIZE + x % CHUNK_SIZE : x % CHUNK_SIZE)
#define CHUNK_FROM_WORLD_COORDS(x) ((x / CHUNK_SIZE < 0 ? x + 1 : x) / CHUNK_SIZE + WORLD_SIZE / 2 - (x < 0 ? 1 : 0))

typedef struct
{
    chunk *chunks[WORLD_SIZE];

    float window_width;
    float window_height;

    vec3 camera_position;
    vec2 camera_rotation;
    int selected_block_x;
    int selected_block_y;
    int selected_block_z;
    int selected_face_x;
    int selected_face_y;
    int selected_face_z;
    int block_in_range;

    block_vertex *chunk_data_buffer;

    mat4 blocks_model;
    mat4 blocks_view;
    mat4 blocks_projection;

    GLuint blocks_texture;
    GLuint blocks_program;
    GLuint blocks_model_location;
    GLuint blocks_view_location;
    GLuint blocks_projection_location;
} world;

void world_init(world *w);
void world_draw(world *w);
void world_destroy(world *w);

block_id world_get_block(world *w, int x, int y, int z);
void world_set_block(world *w, int x, int y, int z, block_id new_block);

#endif