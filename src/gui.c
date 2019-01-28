#include "gui.h"

#include "util.h"

#include <math.h>

void sprite_init(gui_sprite *sprite, gui *g, vec2 tex_position)
{
    glGenBuffers(1, &sprite->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, sprite->vbo);

    glGenVertexArrays(1, &sprite->vao);
    glBindVertexArray(sprite->vao);
    glEnableVertexAttribArray(g->gui_shader.position_location);
    glVertexAttribPointer(g->gui_shader.position_location, 2, GL_FLOAT, GL_FALSE, sizeof(gui_vertex), NULL);
    glEnableVertexAttribArray(g->gui_shader.tex_coord_location);
    glVertexAttribPointer(g->gui_shader.tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(gui_vertex), (GLvoid *) sizeof(vec2));

    gui_vertex buffer_data[4] =
    {
        {{-sprite->size.x * 0.5f, -sprite->size.y * 0.5f}, {tex_position.x / 256.0f, (tex_position.y + sprite->size.y) / 256.0f}},
        {{sprite->size.x * 0.5f, -sprite->size.y * 0.5f}, {(tex_position.x + sprite->size.x) / 256.0f, (tex_position.y + sprite->size.y) / 256.0f}},
        {{sprite->size.x * 0.5f, sprite->size.y * 0.5f}, {(tex_position.x + sprite->size.x) / 256.0f, tex_position.y / 256.0f}},
        {{-sprite->size.x * 0.5f, sprite->size.y * 0.5f}, {tex_position.x / 256.0f, tex_position.y / 256.0f}},
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer_data), buffer_data, GL_STATIC_DRAW);
}

void sprite_draw(gui_sprite *sprite, gui *g)
{
    mat4 model;
    translate_v2(&model, &sprite->position);

    glUniformMatrix4fv(g->gui_shader.model_location, 1, GL_FALSE, model.value);

    glBindVertexArray(sprite->vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void sprite_destroy(gui_sprite *sprite)
{
    glDeleteBuffers(1, &sprite->vbo);
    glDeleteVertexArrays(1, &sprite->vao);
}

void gui_init(gui *g, world *w)
{
    glLogicOp(GL_INVERT);

    g->w = w;

    g->gui_shader.program = load_program("res/shaders/gui.vsh", "res/shaders/gui.fsh");
    g->gui_shader.position_location = glGetAttribLocation(g->gui_shader.program, "position");
    g->gui_shader.tex_coord_location = glGetAttribLocation(g->gui_shader.program, "tex_coord");
    g->gui_shader.projection_location = glGetUniformLocation(g->gui_shader.program, "projection");
    g->gui_shader.model_location = glGetUniformLocation(g->gui_shader.program, "model");
    g->gui_shader.texture_location = glGetUniformLocation(g->gui_shader.program, "gui_texture");

    glGenTextures(1, &g->gui_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g->gui_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("res/textures/widgets.png");

    g->crosshair_sprite.position = (vec2) {0.0f, 0.0f};
    g->crosshair_sprite.size = (vec2) {15.0f, 15.0f};
    sprite_init(&g->crosshair_sprite, g, (vec2) {240.0f, 0.0f});

    g->hotbar_sprite.size = (vec2) {182.0f, 22.0f};
    sprite_init(&g->hotbar_sprite, g, (vec2) {0.0f, 0.0f});

    g->hotbar_selection_sprite.size = (vec2) {24.0f, 24.0f};
    sprite_init(&g->hotbar_selection_sprite, g, (vec2) {0.0f, 22.0f});

    glGenBuffers(256, g->hotbar_item_vbos);
    glGenVertexArrays(256, g->hotbar_item_vaos);

    block_vertex hotbar_item_buffer[36];

    for (int i = 0; i < 256; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, g->hotbar_item_vbos[i]);
        glBindVertexArray(g->hotbar_item_vaos[i]);

        glEnableVertexAttribArray(w->blocks_shader.position_location);
        glVertexAttribPointer(w->blocks_shader.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(block_vertex), NULL);
        glEnableVertexAttribArray(w->blocks_shader.normal_location);
        glVertexAttribPointer(w->blocks_shader.normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(block_vertex), (GLvoid *) sizeof(vec3));
        glEnableVertexAttribArray(w->blocks_shader.tex_coord_location);
        glVertexAttribPointer(w->blocks_shader.tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(block_vertex), (GLvoid *) (sizeof(vec3) * 2));

        vec2 face_tex[6];
        block_id neighbours[6];
        for (int j = 0; j < 6; j++)
        {
            face_tex[j] = (vec2) {blocks[i].face_tiles[j] % 16, blocks[i].face_tiles[j] / 16};
            neighbours[j] = AIR;
        }
        make_block(hotbar_item_buffer, (vec3) {0.0f, 0.0f, 0.0f}, face_tex, neighbours);
        glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(block_vertex), hotbar_item_buffer, GL_STATIC_DRAW);
    }
}

void gui_handle_input(gui *g, input *i)
{
    g->window_width = i->window_width;
    g->window_height = i->window_height;
}

void gui_draw(gui *g)
{
    glUseProgram(g->gui_shader.program);
    glUniform1i(g->gui_shader.texture_location, 1);

    int scale = 1 + 2 * (g->window_height / 720);

    ortho(&TEMP_MAT, -g->window_width / 2 / scale, g->window_width / 2 / scale, -g->window_height / 2 / scale, g->window_height / 2 / scale, -1.0f, 1.0f);
    glUniformMatrix4fv(g->gui_shader.projection_location, 1, GL_FALSE, TEMP_MAT.value);

    glEnable(GL_COLOR_LOGIC_OP);
    sprite_draw(&g->crosshair_sprite, g);
    glDisable(GL_COLOR_LOGIC_OP);

    g->hotbar_sprite.position.y = -g->window_height / 2.0f / scale + g->hotbar_sprite.size.y / 2.0f;
    sprite_draw(&g->hotbar_sprite, g);
    g->hotbar_selection_sprite.position.y = -g->window_height / 2.0f / scale + g->hotbar_sprite.size.y / 2.0f;
    g->hotbar_selection_sprite.position.x = (g->w->selected_block - 1) % 9 * 20.0f - 4 * 20.0f;
    sprite_draw(&g->hotbar_selection_sprite, g);

    glUseProgram(g->w->blocks_shader.program);

    ortho(&TEMP_MAT, -g->window_width / 20 / scale, g->window_width / 20 / scale, -g->window_height / 20 / scale, g->window_height / 20 / scale, -1.0f, 2.0f);
    glUniformMatrix4fv(g->w->blocks_shader.projection_location, 1, GL_FALSE, TEMP_MAT.value);

    identity(&TEMP_MAT);
    glUniformMatrix4fv(g->w->blocks_shader.view_location, 1, GL_FALSE, TEMP_MAT.value);

    mat4 model;

    for (int i = 0; i < 9; i++)
    {
        identity(&model);
        rotate(&TEMP_MAT, &AXIS_UP, RADIANS(-45.0f));
        multiply(&model, &TEMP_MAT, &model);
        rotate(&TEMP_MAT, &AXIS_RIGHT, RADIANS(-30.0f));
        multiply(&model, &TEMP_MAT, &model);
        vec2 translation = {i * 2.0f - 8.0f, -g->window_height / scale / 20.0f + 1.1f};
        translate_v2(&TEMP_MAT, &translation);
        multiply(&model, &TEMP_MAT, &model);

        glUniformMatrix4fv(g->w->blocks_shader.model_location, 1, GL_FALSE, model.value);
        glBindVertexArray(g->hotbar_item_vaos[i + 1 + (g->w->selected_block - 1) / 9 * 9]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void gui_destroy(gui *g)
{
    sprite_destroy(&g->crosshair_sprite);
    sprite_destroy(&g->hotbar_sprite);
    sprite_destroy(&g->hotbar_selection_sprite);
    glDeleteBuffers(256, g->hotbar_item_vbos);
    glDeleteVertexArrays(256, g->hotbar_item_vaos);
    glDeleteTextures(1, &g->gui_texture);
    glDeleteProgram(g->gui_shader.program);
}
