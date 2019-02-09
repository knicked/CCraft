#ifndef MESH_H
#define MESH_H

#include "glmath.h"
#include "block_data.h"

typedef struct
{
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
} block_vertex;

int make_block(block_vertex *data, vec3 position, block_id block, block_id neighbours[6]);
void make_frame(vec3 *data, vec3 *position, bounding_box *box);

#endif
