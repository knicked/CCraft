#include "mesh.h"

#include <glad/glad.h>

int make_block(block_vertex *data, vec3 position, block_id block, block_id neighbours[6])
{
    static vec3 cube_positions[][4] = {
        //front face
        {
            {-0.5f,-0.5f, 0.5f},
            { 0.5f,-0.5f, 0.5f},
            { 0.5f, 0.5f, 0.5f},
            {-0.5f, 0.5f, 0.5f},
        },
        //back face
        {
            { 0.5f,-0.5f,-0.5f},
            {-0.5f,-0.5f,-0.5f},
            {-0.5f, 0.5f,-0.5f},
            { 0.5f, 0.5f,-0.5f},
        },
        //right face
        {
            { 0.5f,-0.5f, 0.5f},
            { 0.5f,-0.5f,-0.5f},
            { 0.5f, 0.5f,-0.5f},
            { 0.5f, 0.5f, 0.5f},
        },
        //left face
        {
            {-0.5f,-0.5f,-0.5f},
            {-0.5f,-0.5f, 0.5f},
            {-0.5f, 0.5f, 0.5f},
            {-0.5f, 0.5f,-0.5f},
        },
        //top face
        {
            {-0.5f, 0.5f, 0.5f},
            { 0.5f, 0.5f, 0.5f},
            { 0.5f, 0.5f,-0.5f},
            {-0.5f, 0.5f,-0.5f},
        },
        //bottom face
        {
            {-0.5f,-0.5f,-0.5f},
            { 0.5f,-0.5f,-0.5f},
            { 0.5f,-0.5f, 0.5f},
            {-0.5f,-0.5f, 0.5f},
        },
    };
    static const vec3 cube_normals[] = {
        //front face
        { 0.0f, 0.0f,-1.0f},
        //back face
        { 0.0f, 0.0f, 1.0f},
        //right face
        { 1.0f, 0.0f, 0.0f},
        //left face
        {-1.0f, 0.0f, 0.0f},
        //top face
        { 0.0f, 1.0f, 0.0f},
        //bottom face
        { 0.0f,-1.0f, 0.0f},
    };
    static const vec2 cube_tex_coords[][4] = {
        //front face
        {
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f},
        },
        //back face
        {
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f},
        },
        //right face
        {
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f},
        },
        //left face
        {
            {1.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
        },
        //top face
        {
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f},
        },
        //bottom face
        {
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f},
        },
    };
    GLushort cube_indices[] = {
        0, 1, 2, 2, 3, 0,
    };
    block_vertex *d = data;
    int vert_count = 0;

    if (block == SAPLING)
    {
        static vec3 cross_positions[][4] = {
            {
                {-0.5f,-0.5f,-0.5f},
                { 0.5f,-0.5f, 0.5f},
                { 0.5f, 0.5f, 0.5f},
                {-0.5f, 0.5f,-0.5f},
            },
            {
                {-0.5f,-0.5f, 0.5f},
                { 0.5f,-0.5f,-0.5f},
                { 0.5f, 0.5f,-0.5f},
                {-0.5f, 0.5f, 0.5f},
            },
        };

        static const vec2 cross_tex_coords[][4] = {
            {
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f},
                {0.0f, 0.0f},
            },
            {
                {1.0f, 1.0f},
                {0.0f, 1.0f},
                {0.0f, 0.0f},
                {1.0f, 0.0f},
            },
        };

        GLushort cross_indices[] = {
            0, 1, 2, 2, 3, 0,
        };

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                float tex_x = blocks[block].face_tiles[0] % 16;
                float tex_y = blocks[block].face_tiles[0] / 16;

                add_v3(&d->position, &position, &cross_positions[i][cross_indices[j]]);
                d->normal = (vec3) {0.0f, 1.0f, 0.0f};
                d->tex_coord.x = tex_x / 16.0f + cross_tex_coords[0][cross_indices[j]].x / 16.0f;
                d->tex_coord.y = tex_y / 16.0f + cross_tex_coords[0][cross_indices[j]].y / 16.0f;
                d++;
                vert_count++;
            }
            for (int j = 5; j >= 0; j--)
            {
                float tex_x = blocks[block].face_tiles[0] % 16;
                float tex_y = blocks[block].face_tiles[0] / 16;

                add_v3(&d->position, &position, &cross_positions[i][cross_indices[j]]);
                d->normal = (vec3) {0.0f, 1.0f, 0.0f};
                d->tex_coord.x = tex_x / 16.0f + cross_tex_coords[1][cross_indices[j]].x / 16.0f;
                d->tex_coord.y = tex_y / 16.0f + cross_tex_coords[1][cross_indices[j]].y / 16.0f;
                d++;
                vert_count++;
            }
        }
    }
    else
    {
        for (int i = 0; i < 6; i++)
        {
            if (!block_is_opaque(neighbours[i]) && (block != GLASS || neighbours[i] != GLASS))
            {
                float tex_x = blocks[block].face_tiles[i] % 16;
                float tex_y = blocks[block].face_tiles[i] / 16;
                for (int j = 0; j < 6; j++)
                {
                    add_v3(&d->position, &position, &cube_positions[i][cube_indices[j]]);
                    d->normal = cube_normals[i];
                    d->tex_coord.x = tex_x / 16.0f + cube_tex_coords[i][cube_indices[j]].x / 16.0f;
                    d->tex_coord.y = tex_y / 16.0f + cube_tex_coords[i][cube_indices[j]].y / 16.0f;
                    d++;
                    vert_count++;
                }
            }
        }
    }
    return vert_count;
}

void make_frame(vec3 *data, vec3 *position, bounding_box *box)
{
    static const vec3 positions[] =
    {
        {-0.5f, 0.5f, 0.5f},
        { 0.5f, 0.5f, 0.5f},
        { 0.5f, 0.5f,-0.5f},
        {-0.5f, 0.5f,-0.5f},

        {-0.5f,-0.5f, 0.5f},
        { 0.5f,-0.5f, 0.5f},
        { 0.5f,-0.5f,-0.5f},
        {-0.5f,-0.5f,-0.5f},
    };

    static const GLushort indices[] =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0,

        0, 4,
        1, 5,
        2, 6,
        3, 7,

        4, 5,
        5, 6,
        6, 7,
        7, 4,
    };

    for (int i = 0; i < 24; i++)
    {
        data->x = positions[indices[i]].x * box->size.x + position->x;
        data->y = positions[indices[i]].y * box->size.y + position->y;
        data->z = positions[indices[i]].z * box->size.z + position->z;
        data++;
    }
}
