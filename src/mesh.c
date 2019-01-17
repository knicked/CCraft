#include "mesh.h"

#include <glad/glad.h>

int make_block(block_vertex *data, vec3 position, vec2 face_tex[6], block_id neighbours[6])
{
    static vec3 positions[][4] = {
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
    static const vec3 normals[] = {
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
    static const vec2 tex_coords[][4] = {
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
    GLushort indices[] = {
        0, 1, 2, 2, 3, 0,
    };
    block_vertex *d = data;
    int vert_count = 0;

    for (int i = 0; i < 6; i++)
    {
        if (!block_is_opaque(neighbours[i]))
        {
            for (int j = 0; j < 6; j++)
            {
                add_v3(&d->position, &position, &positions[i][indices[j]]);
                d->normal = normals[i];
                d->tex_coord.x = face_tex[i].x / 16.0f + tex_coords[i][indices[j]].x / 16.0f;
                d->tex_coord.y = face_tex[i].y / 16.0f + tex_coords[i][indices[j]].y / 16.0f;
                d++;
                vert_count++;
            }
        }
    }
    return vert_count;
}

void make_selection_box(vec3 *data, int x, int y, int z)
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
        data->x = positions[indices[i]].x + x;
        data->y = positions[indices[i]].y + y;
        data->z = positions[indices[i]].z + z;
        data++;
    }
}