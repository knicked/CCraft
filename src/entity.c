#include "entity.h"

#include "world.h"

#include <math.h>

void entity_move(entity *e, void *w, vec3 *delta_pos)
{
    for (int axis = 0; axis < 3; axis++)
    {
        for (int y = roundf(e->box.min.y) - 1; y <= roundf(e->box.max.y) + 1; y++)
        {
            for (int x = roundf(e->box.min.x) - 1; x <= roundf(e->box.max.x) + 1; x++)
            {
                for (int z = roundf(e->box.min.z) - 1; z <= roundf(e->box.max.z) + 1; z++)
                {
                    if (world_get_block(w, x, y, z) == AIR)
                        continue;

                    vec3 block_position = {x, y - 0.5f, z};
                    bounding_box_update(&block_box, &block_position);

                    if (axis == 0)
                    {
                        if (e->box.min.z < block_box.max.z && e->box.max.z > block_box.min.z && e->box.min.x < block_box.max.x && e->box.max.x > block_box.min.x)
                        {
                            if (delta_pos->y > 0.0f && e->box.max.y <= block_box.min.y)
                            {
                                float difference = block_box.min.y - e->box.max.y;
                                if (difference < delta_pos->y)
                                    delta_pos->y = difference;
                            }
                            if (delta_pos->y < 0.0f && e->box.min.y >= block_box.max.y)
                            {
                                float difference = block_box.max.y - e->box.min.y;
                                if (difference > delta_pos->y)
                                    delta_pos->y = difference;
                            }
                        }
                    }
                    else if (axis == 1)
                    {
                        if (e->box.min.z < block_box.max.z && e->box.max.z > block_box.min.z && e->box.min.y < block_box.max.y && e->box.max.y > block_box.min.y)
                        {
                            if (delta_pos->x > 0.0f && e->box.max.x <= block_box.min.x)
                            {
                                float difference = block_box.min.x - e->box.max.x;
                                if (difference < delta_pos->x)
                                    delta_pos->x = difference;
                            }
                            if (delta_pos->x < 0.0f && e->box.min.x >= block_box.max.x)
                            {
                                float difference = block_box.max.x - e->box.min.x;
                                if (difference > delta_pos->x)
                                    delta_pos->x = difference;
                            }
                        }
                    }
                    else
                    {
                        if (e->box.min.x < block_box.max.x && e->box.max.x > block_box.min.x && e->box.min.y < block_box.max.y && e->box.max.y > block_box.min.y)
                        {
                            if (delta_pos->z > 0.0f && e->box.max.z <= block_box.min.z)
                            {
                                float difference = block_box.min.z - e->box.max.z;
                                if (difference < delta_pos->z)
                                    delta_pos->z = difference;
                            }
                            if (delta_pos->z < 0.0f && e->box.min.z >= block_box.max.z)
                            {
                                float difference = block_box.max.z - e->box.min.z;
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
            e->box.min.y += delta_pos->y;
            e->box.max.y += delta_pos->y;
        }
        else if (axis == 1)
        {
            e->box.min.x += delta_pos->x;
            e->box.max.x += delta_pos->x;
        }
    }

    add_v3(&e->position, &e->position, delta_pos);
}