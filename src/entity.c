#include "entity.h"

#include "world.h"

#include <math.h>

void entity_move(entity *e, void *w, vec3 *delta_pos)
{
    vec3 entity_min =
    {
        e->position.x - e->box.size.x / 2.0f,
        e->position.y,
        e->position.z - e->box.size.z / 2.0f
    };

    vec3 entity_max =
    {
        e->position.x + e->box.size.x / 2.0f,
        e->position.y + e->box.size.y,
        e->position.z + e->box.size.z / 2.0f
    };

    for (int axis = 0; axis < 3; axis++)
    {
        for (int y = roundf(entity_min.y) - 1; y <= roundf(entity_max.y) + 1; y++)
        {
            for (int x = roundf(entity_min.x) - 1; x <= roundf(entity_max.x) + 1; x++)
            {
                for (int z = roundf(entity_min.z) - 1; z <= roundf(entity_max.z) + 1; z++)
                {
                    if (world_get_block(w, x, y, z) == AIR)
                        continue;

                    vec3 block_min = 
                    {
                        x - block_box.size.x / 2.0f,
                        y - block_box.size.y / 2.0f,
                        z - block_box.size.z / 2.0f
                    };

                    vec3 block_max =
                    {
                        x + block_box.size.x / 2.0f,
                        y + block_box.size.y / 2.0f,
                        z + block_box.size.z / 2.0f
                    };

                    if (axis == 0)
                    {
                        if (entity_min.z < block_max.z && entity_max.z > block_min.z && entity_min.x < block_max.x && entity_max.x > block_min.x)
                        {
                            if (delta_pos->y > 0.0f && entity_max.y <= block_min.y)
                            {
                                float difference = block_min.y - entity_max.y;
                                if (difference < delta_pos->y)
                                    delta_pos->y = difference;
                            }
                            if (delta_pos->y < 0.0f && entity_min.y >= block_max.y)
                            {
                                float difference = block_max.y - entity_min.y;
                                if (difference > delta_pos->y)
                                    delta_pos->y = difference;
                            }
                        }
                    }
                    else if (axis == 1)
                    {
                        if (entity_min.z < block_max.z && entity_max.z > block_min.z && entity_min.y < block_max.y && entity_max.y > block_min.y)
                        {
                            if (delta_pos->x > 0.0f && entity_max.x <= block_min.x)
                            {
                                float difference = block_min.x - entity_max.x;
                                if (difference < delta_pos->x)
                                    delta_pos->x = difference;
                            }
                            if (delta_pos->x < 0.0f && entity_min.x >= block_max.x)
                            {
                                float difference = block_max.x - entity_min.x;
                                if (difference > delta_pos->x)
                                    delta_pos->x = difference;
                            }
                        }
                    }
                    else
                    {
                        if (entity_min.x < block_max.x && entity_max.x > block_min.x && entity_min.y < block_max.y && entity_max.y > block_min.y)
                        {
                            if (delta_pos->z > 0.0f && entity_max.z <= block_min.z)
                            {
                                float difference = block_min.z - entity_max.z;
                                if (difference < delta_pos->z)
                                    delta_pos->z = difference;
                            }
                            if (delta_pos->z < 0.0f && entity_min.z >= block_max.z)
                            {
                                float difference = block_max.z - entity_min.z;
                                if (difference > delta_pos->z)
                                    delta_pos->z = difference;
                            }
                        }
                    }
                }
            }
        }
        if (axis == 0)
        {
            entity_min.y += delta_pos->y;
            entity_max.y += delta_pos->y;
        }
        else if (axis == 1)
        {
            entity_min.x += delta_pos->x;
            entity_max.x += delta_pos->x;
        }
        else
        {
            entity_min.z += delta_pos->z;
            entity_max.z += delta_pos->z;
        }
    }

    add_v3(&e->position, &e->position, delta_pos);
}