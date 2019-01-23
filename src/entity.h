#ifndef ENTITY_H
#define ENTITY_H

#include "bounding_box.h"

typedef struct
{
    vec3 position;
    vec3 velocity;
    vec3 move_direction;
    int jumping;
    int on_ground;
    bounding_box box;
} entity;

void entity_move(entity *e, void *w, vec3 *delta_pos);

#endif