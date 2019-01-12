#ifndef ENTITY_H
#define ENTITY_H

#include "bounding_box.h"

typedef struct
{
    vec3 position;
    vec3 velocity;
    bounding_box box;
} entity;

#endif