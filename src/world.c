#include "world.h"

#include "util.h"
#include "block_data.h"

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
    w->player.position = (vec3) {0.0f, WORLD_HEIGHT, 0.0f};
    w->player.velocity = (vec3) {0.0f};
    w->player.move_direction = (vec3) {0.0f};
    w->player.jumping = 0;
    w->player.on_ground = 0;
    w->selected_block = 1;
    w->destroying_block = 0;
    w->placing_block = 0;
    w->camera_rotation = (vec2) {0.0f};

    w->blocks_shader.program = load_program("res/shaders/blocks.vsh", "res/shaders/blocks.fsh");
    w->blocks_shader.position_location = glGetAttribLocation(w->blocks_shader.program, "position");
    w->blocks_shader.normal_location = glGetAttribLocation(w->blocks_shader.program, "normal");
    w->blocks_shader.tex_coord_location = glGetAttribLocation(w->blocks_shader.program, "tex_coord");
    w->blocks_shader.model_location = glGetUniformLocation(w->blocks_shader.program, "model");
    w->blocks_shader.view_location = glGetUniformLocation(w->blocks_shader.program, "view");
    w->blocks_shader.projection_location = glGetUniformLocation(w->blocks_shader.program, "projection");
    w->blocks_shader.texture_location = glGetUniformLocation(w->blocks_shader.program, "blocks_texture");

    w->lines_shader.program = load_program("res/shaders/lines.vsh", "res/shaders/lines.fsh");
    w->lines_shader.position_location = glGetAttribLocation(w->lines_shader.program, "position");
    w->lines_shader.view_location = glGetUniformLocation(w->lines_shader.program, "view");
    w->lines_shader.projection_location = glGetUniformLocation(w->lines_shader.program, "projection");

    w->chunk_data_buffer = malloc(36 * CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT * sizeof(block_vertex));

    w->chunks = malloc(WORLD_SIZE * WORLD_SIZE * sizeof(chunk));

    for (int x = 0; x < WORLD_SIZE; x++)
    {
        for (int z = 0; z < WORLD_SIZE; z++)
        {
            chunk_init(&w->chunks[x * WORLD_SIZE + z], x - WORLD_SIZE / 2, z - WORLD_SIZE / 2, &w->blocks_shader);
        }
    }

    glGenTextures(1, &w->blocks_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, w->blocks_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("res/textures/terrain.png");

    glGenVertexArrays(1, &w->frame_vao);
    glBindVertexArray(w->frame_vao);

    glGenBuffers(1, &w->frame_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, w->frame_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 24, NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(w->lines_shader.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), NULL);
    glEnableVertexAttribArray(w->lines_shader.position_location);

    w->num_players = 0;
    w->fly_mode = 0;
}

void world_generate(world *w)
{
    static const int GRASS_LEVEL = 40;

    for (int x = 0; x < WORLD_SIZE; x++)
    {
        for (int z = 0; z < WORLD_SIZE; z++)
        {
            chunk *c = &w->chunks[x * WORLD_SIZE + z];

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
                        else if (y == 0)
                            c->blocks[x][y][z] = BEDROCK;
                        else if (y < 25)
                            c->blocks[x][y][z] = STONE;
                        else if (y < GRASS_LEVEL)
                            c->blocks[x][y][z] = DIRT;
                    }
                }
            }

            c->dirty = 1;
        }
    }
}

void world_handle_input(world *w, input *i)
{
    w->window_width = i->window_width;
    w->window_height = i->window_height;

    if (i->mouse_locked)
    {
        w->camera_rotation.x += i->mouse_delta.y * i->mouse_sensitivity;
        w->camera_rotation.y += i->mouse_delta.x * i->mouse_sensitivity;

        w->camera_rotation.x = w->camera_rotation.x > 89.0f ? 89.0f : w->camera_rotation.x;
        w->camera_rotation.x = w->camera_rotation.x < -89.0f ? -89.0f : w->camera_rotation.x;

        w->player.move_direction = (vec3) {0.0f};

        w->player.jumping = i->keys[GLFW_KEY_SPACE];

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
        if (i->keys_down[GLFW_KEY_F])
        {
            w->fly_mode = !w->fly_mode;
            if (w->fly_mode)
                printf("Fly mode turned on.\n");
            else
                printf("Fly mode turned off.\n");
        }
        if (w->player.move_direction.x != 0.0f || w->player.move_direction.z != 0.0f)
        {
            normalize(&w->player.move_direction);
        }
        if (w->fly_mode)
        {
            if (i->keys[GLFW_KEY_SPACE])
            {
                w->player.move_direction.y += 1.0f;
            }
            if (i->keys[GLFW_KEY_LEFT_SHIFT])
            {
                w->player.move_direction.y -= 1.0f;
            }
        }

        if (i->keys_down[GLFW_KEY_1]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 1;
        if (i->keys_down[GLFW_KEY_2]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 2;
        if (i->keys_down[GLFW_KEY_3]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 3;
        if (i->keys_down[GLFW_KEY_4]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 4;
        if (i->keys_down[GLFW_KEY_5]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 5;
        if (i->keys_down[GLFW_KEY_6]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 6;
        if (i->keys_down[GLFW_KEY_7]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 7;
        if (i->keys_down[GLFW_KEY_8]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 8;
        if (i->keys_down[GLFW_KEY_9]) w->selected_block = GET_CURRENT_HOTBAR(w) * 9 + 9;

        if (i->scroll_delta < 0.0)
        {
            w->selected_block++;
        }
        else if (i->scroll_delta > 0.0)
        {
            w->selected_block--;
        }
        if (w->selected_block == 0) w->selected_block = 3 * 9;
        else if (w->selected_block == 3 * 9 + 1) w->selected_block = 1;

        if (i->mouse_buttons_down[GLFW_MOUSE_BUTTON_LEFT])
        {
            w->destroying_block = 1;
        }
        if (i->mouse_buttons_down[GLFW_MOUSE_BUTTON_RIGHT])
        {
            w->placing_block = 1;
        }
        if (i->mouse_buttons_down[GLFW_MOUSE_BUTTON_MIDDLE])
        {
            block_id selected_block = world_get_block(w, w->selected_block_x, w->selected_block_y, w->selected_block_z);
            if (selected_block != AIR) w->selected_block = selected_block;
        }
    }
}

void world_tick(world *w)
{
    if (w->fly_mode)
    {
        w->player.velocity.x *= 0.91f;
        w->player.velocity.z *= 0.91f;
        w->player.velocity.y *= 0.6f;
    }
    else
    {
        w->player.velocity.y -= 0.08f;
        w->player.velocity.y *= 0.98f;

        if (w->player.jumping && w->player.on_ground)
            w->player.velocity.y += 0.5f;

        w->player.velocity.x *= w->player.on_ground ? 0.6f : 0.91f;
        w->player.velocity.z *= w->player.on_ground ? 0.6f : 0.91f;
    }

    w->block_changed = 0;
    if (w->block_in_range)
    {
        if (w->destroying_block)
        {
            world_set_block(w, w->selected_block_x, w->selected_block_y, w->selected_block_z, AIR);
            w->block_changed = 1;
            w->new_block = AIR;
        }
        if (w->placing_block)
        {
            w->selected_block_x += w->selected_face_x;
            w->selected_block_y += w->selected_face_y;
            w->selected_block_z += w->selected_face_z;

            vec3 position = {w->selected_block_x, w->selected_block_y - 0.5f, w->selected_block_z};
            bounding_box_update(&block_box, &position);

            if (!is_colliding(&block_box, &w->player.box))
            {
                world_set_block(w, w->selected_block_x, w->selected_block_y, w->selected_block_z, w->selected_block);
                w->block_changed = 1;
                w->new_block = w->selected_block;
            }
        }
    }
    w->destroying_block = 0;
    w->placing_block = 0;

    vec3 velocity_change = {0.0f};
    if (w->fly_mode)
    {
        velocity_change.x = w->player.move_direction.x * 0.1f;
        velocity_change.z = w->player.move_direction.z * 0.1f;
        velocity_change.y = w->player.move_direction.y * 0.2f;
    }
    else
        multiply_v3f(&velocity_change, &w->player.move_direction, w->player.on_ground ? 0.1f : 0.02f);
    add_v3(&w->player.velocity, &w->player.velocity, &velocity_change);
}

void world_draw(world *w, double delta_time, double time_since_tick)
{
    double tick_delta_time = delta_time * 20.0f;

    bounding_box_update(&w->player.box, &w->player.position);

    vec3 player_delta;
    multiply_v3f(&player_delta, &w->player.velocity, tick_delta_time);
    entity_move(&w->player, w, &player_delta);

    w->player.on_ground = 0;
    for (int x = roundf(w->player.box.min.x); x <= roundf(w->player.box.max.x); x++)
    {
        for (int z = roundf(w->player.box.min.z); z <= roundf(w->player.box.max.z); z++)
        {
            if (world_get_block(w, x, roundf(w->player.position.y - 0.5f), z) == AIR)
                continue;
            vec3 block_position = {x, roundf(w->player.position.y) - 1.5f, z};
            bounding_box_update(&block_box, &block_position);
            if (is_touching(&w->player.box, &block_box))
            {
                w->player.on_ground = 1;
                break;
            }
        }
        if (w->player.on_ground) break;
    }

    multiply_v3f(&w->player.velocity, &player_delta, 1.0f / tick_delta_time);

    w->camera_position.x = w->player.position.x;
    w->camera_position.z = w->player.position.z;
    w->camera_position.y = w->player.position.y + 1.62f;

    calculate_selected_block(w, 5.0f);

    glClearColor(0.6f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec3 view_translation;
    multiply_v3f(&view_translation, &w->camera_position, -1.0f);
    translate(&w->world_view, &view_translation);
    rotate(&TEMP_MAT, &AXIS_UP, RADIANS(w->camera_rotation.y));
    multiply(&w->world_view, &TEMP_MAT, &w->world_view);
    rotate(&TEMP_MAT, &AXIS_RIGHT, RADIANS(w->camera_rotation.x));
    multiply(&w->world_view, &TEMP_MAT, &w->world_view);

    perspective(&w->world_projection, 85.0f, w->window_width / w->window_height, 0.05f, 1000.0f);

    glUseProgram(w->blocks_shader.program);
    glUniform1i(w->blocks_shader.texture_location, 0);

    glUniformMatrix4fv(w->blocks_shader.projection_location, 1, GL_FALSE, w->world_projection.value);
    glUniformMatrix4fv(w->blocks_shader.view_location, 1, GL_FALSE, w->world_view.value);

    for (int i = 0; i < 4; i++)
    {
        chunk *chunk_to_update = NULL;
        float min_distance = -1.0f;

        for (int x = 0; x < WORLD_SIZE; x++)
        {
            for (int z = 0; z < WORLD_SIZE; z++)
            {
                chunk *c = &w->chunks[x * WORLD_SIZE + z];

                if (c->dirty)
                {
                    float distance_x = abs(x - CHUNK_FROM_WORLD_COORDS((int) roundf(w->player.position.x)));
                    float distance_z = abs(z - CHUNK_FROM_WORLD_COORDS((int) roundf(w->player.position.z)));
                    float distance = sqrtf(distance_x * distance_x + distance_z * distance_z);
                    if (distance < min_distance || min_distance < 0.0f)
                    {
                        min_distance = distance;
                        chunk_to_update = c;
                    }
                }
            }
        }

        if (chunk_to_update)
        {
            glBindBuffer(GL_ARRAY_BUFFER, chunk_to_update->vbo);
            chunk_build_buffer(chunk_to_update, w, w->chunk_data_buffer);
        }
        else break;
    }

    for (int x = -WORLD_SIZE / 2; x < WORLD_SIZE - WORLD_SIZE / 2; x++)
    {
        for (int z = -WORLD_SIZE / 2; z < WORLD_SIZE - WORLD_SIZE / 2; z++)
        {
            chunk *c = &w->chunks[(x + WORLD_SIZE / 2) * WORLD_SIZE + z + WORLD_SIZE / 2];
            vec3 chunk_translation = {x * CHUNK_SIZE, 0.0f, z * CHUNK_SIZE};
            translate(&w->blocks_model, &chunk_translation);
            glUniformMatrix4fv(w->blocks_shader.model_location, 1, GL_FALSE, w->blocks_model.value);
            glBindVertexArray(c->vao);
            glDrawArrays(GL_TRIANGLES, 0, c->vert_count);
        }
    }

    for (int x = -WORLD_SIZE / 2; x < WORLD_SIZE - WORLD_SIZE / 2; x++)
    {
        for (int z = -WORLD_SIZE / 2; z < WORLD_SIZE - WORLD_SIZE / 2; z++)
        {
            chunk *c = &w->chunks[(x + WORLD_SIZE / 2) * WORLD_SIZE + z + WORLD_SIZE / 2];
            vec3 chunk_translation = {x * CHUNK_SIZE, 0.0f, z * CHUNK_SIZE};
            translate(&w->blocks_model, &chunk_translation);
            glUniformMatrix4fv(w->blocks_shader.model_location, 1, GL_FALSE, w->blocks_model.value);
            glBindVertexArray(c->vao);
            glDrawArrays(GL_TRIANGLES, c->water_offset, c->water_count);
        }
    }

    glUseProgram(w->lines_shader.program);

    glUniformMatrix4fv(w->lines_shader.projection_location, 1, GL_FALSE, w->world_projection.value);
    glUniformMatrix4fv(w->lines_shader.view_location, 1, GL_FALSE, w->world_view.value);

    glBindBuffer(GL_ARRAY_BUFFER, w->frame_vbo);
    glBindVertexArray(w->frame_vao);

    vec3 data[24];
    vec3 position;
    vec3 prev_position;

    for (int i = 0; i < w->num_players; i++)
    {
        position = (vec3)
        {
            w->players[i].x / 32.0f,
            w->players[i].y / 32.0f + w->player.box.size.y * 0.5f,
            w->players[i].z / 32.0f
        };

        prev_position = (vec3)
        {
            w->players[i].prev_x / 32.0f,
            w->players[i].prev_y / 32.0f + w->player.box.size.y * 0.5f,
            w->players[i].prev_z / 32.0f
        };

        lerp_v3(&position, &prev_position, &position, time_since_tick * 20.0f);

        make_frame(data, &position, &w->player.box);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
        glDrawArrays(GL_LINES, 0, 24);
    }

    if (w->block_in_range)
    {
        position = (vec3)
        {
            w->selected_block_x,
            w->selected_block_y,
            w->selected_block_z
        };
        make_frame(data, &position, &block_box);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
        glDrawArrays(GL_LINES, 0, 24);
    }
}

void world_destroy(world *w)
{
    for (int x = 0; x < WORLD_SIZE; x++)
    {
        for (int z = 0; z < WORLD_SIZE; z++)
        {
            chunk_destroy(&w->chunks[x * WORLD_SIZE + z]);
        }
    }
    free(w->chunks);
    free(w->chunk_data_buffer);

    glDeleteBuffers(1, &w->frame_vbo);
    glDeleteVertexArrays(1, &w->frame_vao);

    glDeleteTextures(1, &w->blocks_texture);
    glDeleteProgram(w->blocks_shader.program);
}

block_id world_get_block(world *w, int x, int y, int z)
{
    size_t chunk_x = CHUNK_FROM_WORLD_COORDS(x);
    size_t chunk_z = CHUNK_FROM_WORLD_COORDS(z);
    if (chunk_x < 0 || chunk_x >= WORLD_SIZE || chunk_z < 0 || chunk_z >= WORLD_SIZE || y < 0 || y >= WORLD_HEIGHT)
        return AIR;
    else
        return w->chunks[chunk_x * WORLD_SIZE + chunk_z].blocks[WORLD_TO_CHUNK(x)][y][WORLD_TO_CHUNK(z)];
}

void world_set_block(world *w, int x, int y, int z, block_id new_block)
{
    size_t chunk_x = CHUNK_FROM_WORLD_COORDS(x);
    size_t chunk_z = CHUNK_FROM_WORLD_COORDS(z);
    chunk *c = &w->chunks[chunk_x * WORLD_SIZE + chunk_z];
    
    size_t block_x = WORLD_TO_CHUNK(x);
    size_t block_z = WORLD_TO_CHUNK(z);
    block_id *b = &c->blocks[block_x][y][block_z];

    if (*b != new_block)
    {
        *b = new_block;
        if (block_x == 0) w->chunks[(chunk_x == 0 ? chunk_x : chunk_x - 1) * WORLD_SIZE + chunk_z].dirty = 1;
        else if (block_x == CHUNK_SIZE - 1) w->chunks[(chunk_x == WORLD_SIZE - 1 ? chunk_x : chunk_x + 1) * WORLD_SIZE + chunk_z].dirty = 1;
        if (block_z == 0) w->chunks[chunk_x * WORLD_SIZE + (chunk_z == 0 ? chunk_z : chunk_z - 1)].dirty = 1;
        else if (block_z == CHUNK_SIZE - 1) w->chunks[chunk_x * WORLD_SIZE + (chunk_z == WORLD_SIZE - 1 ? chunk_z : chunk_z + 1)].dirty = 1;
        c->dirty = 1;
    }
}
