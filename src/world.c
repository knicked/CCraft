#include "world.h"

#include "util.h"

#include <stdlib.h>
#include <stddef.h>

void world_init(world *w)
{
    w->camera_position = (vec3){0.0f, 102.0f, 0.0f};
    w->camera_rotation = (vec2){0.0f, 0.0f};

    w->blocks_program = load_program("res/shaders/blocks.vsh", "res/shaders/blocks.fsh");
    w->blocks_model_location = glGetUniformLocation(w->blocks_program, "model");
    w->blocks_view_location = glGetUniformLocation(w->blocks_program, "view");
    w->blocks_projection_location = glGetUniformLocation(w->blocks_program, "projection");

    w->lines_program = load_program("res/shaders/lines.vsh", "res/shaders/lines.fsh");
    w->lines_view_location = glGetUniformLocation(w->lines_program, "view");
    w->lines_projection_location = glGetUniformLocation(w->lines_program, "projection");

    w->chunk_data_buffer = malloc(36 * CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT * sizeof(block_vertex));

    for (int x = 0; x < WORLD_SIZE; x++)
    {
        w->chunks[x] = malloc(WORLD_SIZE * sizeof(chunk));
        for (int z = 0; z < WORLD_SIZE; z++)
        {
            chunk_init(&w->chunks[x][z], x - WORLD_SIZE / 2, z - WORLD_SIZE / 2);
        }
    }

    glGenTextures(1, &w->blocks_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, w->blocks_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("res/textures/terrain.png");

    glGenVertexArrays(1, &w->selection_box_vao);
    glBindVertexArray(w->selection_box_vao);

    glGenBuffers(1, &w->selection_box_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, w->selection_box_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 24, NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, NULL);
    glEnableVertexAttribArray(0);
}

void world_draw(world *w)
{
    glClearColor(0.6f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 temp;

    vec3 view_translation;
    multiply_v3f(&view_translation, &w->camera_position, -1.0f);
    translate(&w->world_view, &view_translation);
    static vec3 axis_up = {0.0f, 1.0f, 0.0f};
    static vec3 axis_right = {1.0f, 0.0f, 0.0f};
    rotate(&temp, &axis_up, RADIANS(w->camera_rotation.y));
    multiply(&w->world_view, &temp, &w->world_view);
    rotate(&temp, &axis_right, RADIANS(w->camera_rotation.x));
    multiply(&w->world_view, &temp, &w->world_view);

    perspective(&w->world_projection, 85.0f, w->window_width / w->window_height, 0.05f, 1000.0f);

    glUseProgram(w->blocks_program);

    glUniformMatrix4fv(w->blocks_projection_location, 1, GL_FALSE, w->world_projection.value);
    glUniformMatrix4fv(w->blocks_view_location, 1, GL_FALSE, w->world_view.value);

    for (int x = -WORLD_SIZE / 2; x < WORLD_SIZE - WORLD_SIZE / 2; x++)
    {
        for (int z = -WORLD_SIZE / 2; z < WORLD_SIZE - WORLD_SIZE / 2; z++)
        {
            chunk *c = &w->chunks[x + WORLD_SIZE / 2][z + WORLD_SIZE / 2];
            vec3 chunk_translation = {x * CHUNK_SIZE, 0.0f, z * CHUNK_SIZE};
            translate(&w->blocks_model, &chunk_translation);
            glUniformMatrix4fv(w->blocks_model_location, 1, GL_FALSE, w->blocks_model.value);
            if (c->dirty) 
            {
                glBindBuffer(GL_ARRAY_BUFFER, c->buffer);
                chunk_build_buffer(c, w, w->chunk_data_buffer);
            }
            glBindVertexArray(c->vao);
            glDrawArrays(GL_TRIANGLES, 0, c->vert_count);
        }
    }

    if (w->block_in_range)
    {
        glUseProgram(w->lines_program);

        glUniformMatrix4fv(w->lines_projection_location, 1, GL_FALSE, w->world_projection.value);
        glUniformMatrix4fv(w->lines_view_location, 1, GL_FALSE, w->world_view.value);

        vec3 data[24];
        make_selection_box(data, w->selected_block_x, w->selected_block_y, w->selected_block_z);

        glBindBuffer(GL_ARRAY_BUFFER, w->selection_box_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);

        glBindVertexArray(w->selection_box_vao);
        glDrawArrays(GL_LINES, 0, 24);
    }
}

void world_destroy(world *w)
{
    for (int x = 0; x < WORLD_SIZE; x++)
    {
        for (int z = 0; z < WORLD_SIZE; z++)
        {
            chunk_destroy(&w->chunks[x][z]);
        }
        free(w->chunks[x]);
    }
    free(w->chunk_data_buffer);

    glDeleteBuffers(1, &w->selection_box_buffer);
    glDeleteVertexArrays(1, &w->selection_box_vao);   

    glDeleteTextures(1, &w->blocks_texture);
    glDeleteProgram(w->blocks_program);
}

block_id world_get_block(world *w, int x, int y, int z)
{
    size_t chunk_x = CHUNK_FROM_WORLD_COORDS(x);
    size_t chunk_z = CHUNK_FROM_WORLD_COORDS(z);
    if (chunk_x < 0 || chunk_x >= WORLD_SIZE || chunk_z < 0 || chunk_z >= WORLD_SIZE || y < 0 || y >= WORLD_HEIGHT)
        return AIR;
    else
        return w->chunks[chunk_x][chunk_z].blocks[WORLD_TO_CHUNK(x)][y][WORLD_TO_CHUNK(z)];
}

void world_set_block(world *w, int x, int y, int z, block_id new_block)
{
    size_t chunk_x = CHUNK_FROM_WORLD_COORDS(x);
    size_t chunk_z = CHUNK_FROM_WORLD_COORDS(z);
    chunk *c = &w->chunks[chunk_x][chunk_z];
    
    size_t block_x = WORLD_TO_CHUNK(x);
    size_t block_z = WORLD_TO_CHUNK(z);
    block_id *b = &c->blocks[block_x][y][block_z];

    if (*b != new_block)
    {
        *b = new_block;
        if (block_x == 0) w->chunks[chunk_x == 0 ? chunk_x : chunk_x - 1][chunk_z].dirty = 1;
        else if (block_x == CHUNK_SIZE - 1) w->chunks[chunk_x == WORLD_SIZE - 1 ? chunk_x : chunk_x + 1][chunk_z].dirty = 1;
        if (block_z == 0) w->chunks[chunk_x][chunk_z == 0 ? chunk_z : chunk_z - 1].dirty = 1;
        else if (block_z == CHUNK_SIZE - 1) w->chunks[chunk_x][chunk_z == WORLD_SIZE - 1 ? chunk_z : chunk_z + 1].dirty = 1;
        c->dirty = 1;
    }
}