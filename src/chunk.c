#include "chunk.h"
#include "world.h"

#include <glad/glad.h>
#include <stddef.h>

void chunk_build_buffer(chunk *c, void *w, block_vertex *data_buffer)
{
    c->vert_count = 0;

    vec2 face_tex[6];
    block_id neighbours[6];

    int x_off = CHUNK_SIZE * c->x;
    int z_off = CHUNK_SIZE * c->z;

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < WORLD_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                if (c->blocks[x][y][z] != AIR)
                {
                    neighbours[0] = world_get_block((world *) w, x + x_off, y, z + z_off + 1);
                    neighbours[1] = world_get_block((world *) w, x + x_off, y, z + z_off - 1);
                    neighbours[2] = world_get_block((world *) w, x + x_off + 1, y, z + z_off);
                    neighbours[3] = world_get_block((world *) w, x + x_off - 1, y, z + z_off);
                    neighbours[4] = world_get_block((world *) w, x + x_off, y + 1, z + z_off);
                    neighbours[5] = world_get_block((world *) w, x + x_off, y - 1, z + z_off);

                    for (int i = 0; i < 6; i++)
                        face_tex[i] = (vec2) {blocks[c->blocks[x][y][z]].face_tiles[i] % 16, blocks[c->blocks[x][y][z]].face_tiles[i] / 16};
                    c->vert_count += make_block(data_buffer + c->vert_count, (vec3){x, y, z}, face_tex, neighbours);
                }
            }
        }
    }

    glBufferData(GL_ARRAY_BUFFER, c->vert_count * sizeof(block_vertex), data_buffer, GL_STATIC_DRAW);

    c->dirty = 0;
}

void chunk_init(chunk *c, int x, int z, GLuint position_location, GLuint normal_location, GLuint tex_coord_location)
{
    glGenBuffers(1, &c->buffer);
    glBindBuffer(GL_ARRAY_BUFFER, c->buffer);

    glGenVertexArrays(1, &c->vao);
    glBindVertexArray(c->vao);
    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, NULL);
    glEnableVertexAttribArray(normal_location);
    glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (GLvoid *) (sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(tex_coord_location);
    glVertexAttribPointer(tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (GLvoid *) (sizeof(GLfloat) * 6));

    static const int GRASS_LEVEL = 100;

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < WORLD_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                if (y > GRASS_LEVEL)
                    c->blocks[x][y][z] = AIR;
                else if (y == GRASS_LEVEL)
                    c->blocks[x][y][z] = GRASS;
                else if (y < GRASS_LEVEL)
                    c->blocks[x][y][z] = DIRT;
                if (y < 50)
                    c->blocks[x][y][z] = STONE;
            }
        }
    }

    c->x = x;
    c->z = z;
}

void chunk_destroy(chunk *c)
{
    glDeleteBuffers(1, &c->buffer);
    glDeleteVertexArrays(1, &c->vao);
}