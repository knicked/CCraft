#include "block_data.h"

const block_data blocks[256] = {
    {0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1}, //stone
    {3, 3, 3, 3, 0, 2}, //grass
    {2, 2, 2, 2, 2, 2}, //dirt
    {16, 16, 16, 16, 16, 16}, //cobblestone
    {4, 4, 4, 4, 4, 4}, //wooden planks
    {15, 15, 15, 15, 15, 15}, //sapling
    {17, 17, 17, 17, 17, 17}, //bedrock
    {205, 205, 205, 205, 205, 205}, //water
    {205, 205, 205, 205, 205, 205}, //also water
    {237, 237, 237, 237, 237, 237}, //lava
    {237, 237, 237, 237, 237, 237}, //also lava
    {18, 18, 18, 18, 18, 18}, //sand
    {19, 19, 19, 19, 19, 19}, //gravel
    {32, 32, 32, 32, 32, 32}, //gold ore
    {33, 33, 33, 33, 33, 33}, //iron ore
    {34, 34, 34, 34, 34, 34}, //coal ore
    {20, 20, 20, 20, 21, 21}, //wood
    {52, 52, 52, 52, 52, 52}, //leaves
    {48, 48, 48, 48, 48, 48}, //sponge
    {49, 49, 49, 49, 49, 49}, //glass
};

bounding_box block_box = {{1.0f, 1.0f, 1.0f}};

int block_is_opaque(block_id block)
{
    switch (block)
    {
        case AIR:
        case FLOWING_WATER:
        case STILL_WATER:
        case LEAVES:
        case SAPLING:
        case GLASS:
            return 0;
        default:
            return 1;
    }
}

int block_is_obstacle(block_id block)
{
    switch (block)
    {
        case AIR:
        case FLOWING_WATER:
        case STILL_WATER:
        case FLOWING_LAVA:
        case STILL_LAVA:
        case SAPLING:
            return 0;
        default:
            return 1;
    }
}

int block_connects(block_id block)
{
    switch (block)
    {
        case FLOWING_WATER:
        case STILL_WATER:
        case GLASS:
            return 1;
        default:
            return 0;
    }
}

int block_can_be_placed_on(block_id ground, block_id block)
{
    switch (block)
    {
        case SAPLING:
            return ground == GRASS;
        default:
            return 1;
    }
}
