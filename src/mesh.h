#ifndef MESH_H
#define MESH_H

#include "glmath.h"
#include "block.h"

typedef struct
{
    vec3 position;
    vec2 tex_coord;
} block_vertex;

int make_block(block_vertex *data, vec3 position, vec2 face_tex[6], block_id neighbours[6]);

#endif