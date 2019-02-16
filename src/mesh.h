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

typedef struct
{
    vec2 position;
    vec2 tex_coord;
    float tex_id;
    vec3 color;
} gui_vertex;

int make_block(block_vertex *data, vec3 position, block_id block, block_id neighbours[6]);
void make_frame(vec3 *data, vec3 *position, bounding_box *box);
int make_text(gui_vertex *data, const char *text, float scale, vec2 *size);

#endif
