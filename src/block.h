#ifndef BLOCK_H
#define BLOCK_H

#include "glmath.h"

#define AIR 0
#define STONE 1
#define GRASS 2
#define DIRT 3
#define COBBLESTONE 4
#define WOOD_PLANKS 5

typedef unsigned char block_id;

typedef struct
{
    int face_tiles[6];
} block_data;

extern const block_data blocks[256];

#endif