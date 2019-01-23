#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "glmath.h"

typedef struct
{
    vec3 size;
    vec3 max;
    vec3 min;
} bounding_box;

int bounding_box_update(bounding_box *box, vec3 *position);

int is_touching(bounding_box *b1, bounding_box *b2);

#endif