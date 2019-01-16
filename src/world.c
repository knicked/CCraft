#include "world.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

void calculate_selected_block(world *w, float radius)
{
    vec3 direction =
    {
        .x = -sinf(RADIANS(w->camera_rotation.y)) * cosf(RADIANS(w->camera_rotation.x)),
        .z = -cosf(RADIANS(w->camera_rotation.y)) * cosf(RADIANS(w->camera_rotation.x)),
        .y = sinf(RADIANS(w->camera_rotation.x))
    };

    w->selected_block_x = roundf(w->camera_position.x);
    w->selected_block_z = roundf(w->camera_position.z);
    w->selected_block_y = roundf(w->camera_position.y);

    int step_x = direction.x > 0.0f ? 1 : direction.x < 0.0f ? -1 : 0;
    int step_z = direction.z > 0.0f ? 1 : direction.z < 0.0f ? -1 : 0;
    int step_y = direction.y > 0.0f ? 1 : direction.y < 0.0f ? -1 : 0;

    float t_max_x = (direction.x > 0.0f ? roundf(w->camera_position.x) + 0.5f - w->camera_position.x : roundf(w->camera_position.x) - 0.5f - w->camera_position.x) / direction.x;
    float t_max_z = (direction.z > 0.0f ? roundf(w->camera_position.z) + 0.5f - w->camera_position.z : roundf(w->camera_position.z) - 0.5f - w->camera_position.z) / direction.z;
    float t_max_y = (direction.y > 0.0f ? roundf(w->camera_position.y) + 0.5f - w->camera_position.y : roundf(w->camera_position.y) - 0.5f - w->camera_position.y) / direction.y;

    float t_delta_x = (float) step_x / direction.x;
    float t_delta_z = (float) step_z / direction.z;
    float t_delta_y = (float) step_y / direction.y;

    while (1)
    {
        if (t_max_x < t_max_y)
        {
            if (t_max_x < t_max_z)
            {
                if (t_max_x > radius)
                {
                    w->block_in_range = 0;
                    return;
                }
                w->selected_block_x += step_x;
                t_max_x += t_delta_x;

                w->selected_face_x = -step_x;
                w->selected_face_y = 0;
                w->selected_face_z = 0;
            }
            else
            {
                if (t_max_z > radius)
                {
                    w->block_in_range = 0;
                    return;
                }
                w->selected_block_z += step_z;
                t_max_z += t_delta_z;

                w->selected_face_x = 0;
                w->selected_face_y = 0;
                w->selected_face_z = -step_z;
            }
        }
        else
        {
            if (t_max_y < t_max_z)
            {
                if (t_max_y > radius)
                {
                    w->block_in_range = 0;
                    return;
                }
                w->selected_block_y += step_y;
                t_max_y += t_delta_y;

                w->selected_face_x = 0;
                w->selected_face_y = -step_y;
                w->selected_face_z = 0;
            }
            else
            {
                if (t_max_z > radius)
                {
                    w->block_in_range = 0;
                    return;
                }
                w->selected_block_z += step_z;
                t_max_z += t_delta_z;

                w->selected_face_x = 0;
                w->selected_face_y = 0;
                w->selected_face_z = -step_z;
            }
        }
        if (world_get_block(w, w->selected_block_x, w->selected_block_y, w->selected_block_z) != AIR)
        {
            w->block_in_range = 1;
            return;
        }
    }
}

void world_init(world *w)
{
    w->player.box = (bounding_box) {{0.6f, 1.8f, 0.6f}};
    w->player.position = (vec3) {0.0f, 100.5f, 0.0f};
    w->player.velocity = (vec3) {0.0f};
    w->player.move_direction = (vec3) {0.0f};
    w->selected_block = 1;
    w->destroying_block = 0;
    w->placing_block = 0;
    w->camera_rotation = (vec2) {0.0f};

    w->blocks_program = load_program("res/shaders/blocks.vsh", "res/shaders/blocks.fsh");
    w->blocks_position_location = glGetAttribLocation(w->blocks_program, "position");
    w->blocks_normal_location = glGetAttribLocation(w->blocks_program, "normal");
    w->blocks_tex_coord_location = glGetAttribLocation(w->blocks_program, "texCoord");
    w->blocks_model_location = glGetUniformLocation(w->blocks_program, "model");
    w->blocks_view_location = glGetUniformLocation(w->blocks_program, "view");
    w->blocks_projection_location = glGetUniformLocation(w->blocks_program, "projection");

    w->lines_program = load_program("res/shaders/lines.vsh", "res/shaders/lines.fsh");
    w->lines_position_location = glGetAttribLocation(w->lines_program, "position");
    w->lines_view_location = glGetUniformLocation(w->lines_program, "view");
    w->lines_projection_location = glGetUniformLocation(w->lines_program, "projection");

    w->chunk_data_buffer = malloc(36 * CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT * sizeof(block_vertex));

    for (int x = 0; x < WORLD_SIZE; x++)
    {
        w->chunks[x] = malloc(WORLD_SIZE * sizeof(chunk));
        for (int z = 0; z < WORLD_SIZE; z++)
        {
            chunk_init(&w->chunks[x][z], x - WORLD_SIZE / 2, z - WORLD_SIZE / 2, w->blocks_position_location, w->blocks_normal_location, w->blocks_tex_coord_location);
        }
    }

    for (int x = 0; x < WORLD_SIZE; x++)
    {
        for (int z = 0; z < WORLD_SIZE; z++)
        {
            glBindBuffer(GL_ARRAY_BUFFER, w->chunks[x][z].buffer);
            chunk_build_buffer(&w->chunks[x][z], w, w->chunk_data_buffer);
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
    glVertexAttribPointer(w->lines_position_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, NULL);
    glEnableVertexAttribArray(w->lines_position_location);
}

void world_handle_input(world *w, input *i)
{
    if (i->mouse_locked)
    {
        w->camera_rotation.x += i->mouse_delta.y * i->mouse_sensitivity;
        w->camera_rotation.y += i->mouse_delta.x * i->mouse_sensitivity;

        w->camera_rotation.x = w->camera_rotation.x > 89.0f ? 89.0f : w->camera_rotation.x;
        w->camera_rotation.x = w->camera_rotation.x < -89.0f ? -89.0f : w->camera_rotation.x;

        w->player.move_direction = (vec3) {0.0f};

        if (i->keys[GLFW_KEY_SPACE])
            w->player.move_direction.y += 1.0f;
        if (i->keys[GLFW_KEY_LEFT_SHIFT])
            w->player.move_direction.y -= 1.0f;

        if (i->keys[GLFW_KEY_W])
        {
            w->player.move_direction.x -= sinf(RADIANS(w->camera_rotation.y));
            w->player.move_direction.z -= cosf(RADIANS(w->camera_rotation.y));
        }
        if (i->keys[GLFW_KEY_A])
        {
            w->player.move_direction.x -= cosf(RADIANS(w->camera_rotation.y));
            w->player.move_direction.z += sinf(RADIANS(w->camera_rotation.y));
        }
        if (i->keys[GLFW_KEY_S])
        {
            w->player.move_direction.x += sinf(RADIANS(w->camera_rotation.y));
            w->player.move_direction.z += cosf(RADIANS(w->camera_rotation.y));
        }
        if (i->keys[GLFW_KEY_D])
        {
            w->player.move_direction.x += cosf(RADIANS(w->camera_rotation.y));
            w->player.move_direction.z -= sinf(RADIANS(w->camera_rotation.y));
        }

        if (w->player.move_direction.x != 0.0f || w->player.move_direction.y != 0.0f || w->player.move_direction.z != 0.0f)
        {
            normalize(&w->player.move_direction);
        }

        if (i->scroll_delta != 0.0)
        {
            w->selected_block += i->scroll_delta;
            printf("Selected block of ID %d\n", w->selected_block);
        }

        if (i->mouse_buttons_down[GLFW_MOUSE_BUTTON_LEFT])
        {
            w->destroying_block = 1;
        }
        if (i->mouse_buttons_down[GLFW_MOUSE_BUTTON_RIGHT])
        {
            w->placing_block = 1;      
        }
    }
}

void world_tick(world *w)
{
    multiply_v3f(&w->player.velocity, &w->player.velocity, 0.6f);

    if (w->block_in_range)
    {
        if (w->destroying_block)
        {
            world_set_block(w, w->selected_block_x, w->selected_block_y, w->selected_block_z, AIR);
        }
        if (w->placing_block)
        {
            world_set_block(w,
                w->selected_block_x + w->selected_face_x,
                w->selected_block_y + w->selected_face_y,
                w->selected_block_z + w->selected_face_z, w->selected_block);
        }
    }
    w->destroying_block = 0;
    w->placing_block = 0;

    vec3 velocity_change = {0.0f};
    multiply_v3f(&velocity_change, &w->player.move_direction, 0.2f);
    add_v3(&w->player.velocity, &w->player.velocity, &velocity_change);
}

void world_draw(world *w, double delta_time)
{
    double tick_delta_time = delta_time * 20.0f;

    vec3 player_delta;
    multiply_v3f(&player_delta, &w->player.velocity, tick_delta_time);

    entity_move(&w->player, w, &player_delta);

    multiply_v3f(&w->player.velocity, &player_delta, 1.0f / tick_delta_time);

    w->camera_position.x = w->player.position.x;
    w->camera_position.z = w->player.position.z;
    w->camera_position.y = w->player.position.y + 1.62f;

    calculate_selected_block(w, 5.0f);

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