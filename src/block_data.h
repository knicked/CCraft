#ifndef BLOCK_DATA_H
#define BLOCK_DATA_H

#include "block.h"
#include "bounding_box.h"

typedef struct
{
    int face_tiles[6];
} block_data;

extern const block_data blocks[256];
extern bounding_box block_box;

int block_is_opaque(block_id block);
int block_is_obstacle(block_id block);

#endif
