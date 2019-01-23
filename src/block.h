#ifndef BLOCK_H
#define BLOCK_H

#include "bounding_box.h"

#define AIR 0
#define STONE 1
#define GRASS 2
#define DIRT 3
#define COBBLESTONE 4
#define WOOD_PLANKS 5
#define SAPLING 6
#define BEDROCK 7
#define FLOWING_WATER 8
#define STILL_WATER 9
#define FLOWING_LAVA 10
#define STILL_LAVA 11
#define SAND 12
#define GRAVEL 13
#define GOLD_ORE 14
#define IRON_ORE 15
#define COAL_ORE 16
#define WOOD 17
#define LEAVES 18
#define SPONGE 19
#define GLASS 20

typedef unsigned char block_id;

typedef struct
{
    int face_tiles[6];
} block_data;

extern const block_data blocks[256];
extern bounding_box block_box;

int block_is_opaque(block_id block);

#endif