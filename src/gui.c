#include "gui.h"

#include "util.h"

#include <math.h>

void sprite_init(gui_sprite *sprite, gui *g, vec2 tex_position)
{
    glGenBuffers(1, &sprite->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, sprite->vbo);

    glGenVertexArrays(1, &sprite->vao);
    glBindVertexArray(sprite->vao);
    glEnableVertexAttribArray(g->gui_position_location);
    glVertexAttribPointer(g->gui_position_location, 2, GL_FLOAT, GL_FALSE, sizeof(gui_vertex), NULL);
    glEnableVertexAttribArray(g->gui_tex_coord_location);
    glVertexAttribPointer(g->gui_tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(gui_vertex), (GLvoid *) sizeof(vec2));

    gui_vertex buffer_data[4] =
    {
        {{-sprite->size.x * 0.5f, -sprite->size.y * 0.5f}, {tex_position.x / 256.0f, (tex_position.y + sprite->size.y) / 256.0f}},
        {{sprite->size.x * 0.5f, -sprite->size.y * 0.5f}, {(tex_position.x + sprite->size.x) / 256.0f, (tex_position.y + sprite->size.y) / 256.0f}},
        {{sprite->size.x * 0.5f, sprite->size.y * 0.5f}, {(tex_position.x + sprite->size.x) / 256.0f, tex_position.y}},
        {{-sprite->size.x * 0.5f, sprite->size.y * 0.5f}, {tex_position.x / 256.0f, tex_position.y / 256.0f}},
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer_data), buffer_data, GL_STATIC_DRAW);
}

void sprite_draw(gui_sprite *sprite, gui *g)
{
    mat4 model;
    translate_v2(&model, &sprite->position);

    glUniformMatrix4fv(g->gui_model_location, 1, GL_FALSE, model.value);

    glBindVertexArray(sprite->vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void sprite_destroy(gui_sprite *sprite)
{
    glDeleteBuffers(1, &sprite->vbo);
    glDeleteVertexArrays(1, &sprite->vao);
}

void gui_init(gui *g)
{
    glLogicOp(GL_INVERT);

    g->gui_program = load_program("res/shaders/gui.vsh", "res/shaders/gui.fsh");
    g->gui_position_location = glGetAttribLocation(g->gui_program, "position");
    g->gui_tex_coord_location = glGetAttribLocation(g->gui_program, "tex_coord");
    g->gui_projection_location = glGetUniformLocation(g->gui_program, "projection");
    g->gui_model_location = glGetUniformLocation(g->gui_program, "model");

    glGenTextures(1, &g->gui_texture);
    glBindTexture(GL_TEXTURE_2D, g->gui_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("res/textures/widgets.png");

    g->crosshair_sprite.position = (vec2) {0.0f, 0.0f};
    g->crosshair_sprite.size = (vec2) {15.0f, 15.0f};
    sprite_init(&g->crosshair_sprite, g, (vec2) {240.0f, 0.0f});

    g->hotbar_sprite.position = (vec2) {0.0f, -50.0f};
    g->hotbar_sprite.size = (vec2) {182.0f, 22.0f};
    sprite_init(&g->hotbar_sprite, g, (vec2) {0.0f, 0.0f});
}

void gui_handle_input(gui *g, input *i)
{
    g->window_width = i->window_width;
    g->window_height = i->window_height;
}

void gui_draw(gui *g)
{
    glUseProgram(g->gui_program);
    glBindTexture(GL_TEXTURE_2D, g->gui_texture);

    int scale = 1 + 2 * (g->window_height / 720);

    mat4 projection;
    ortho(&projection, -g->window_width / 2 / scale, g->window_width / 2 / scale, -g->window_height / 2 / scale, g->window_height / 2 / scale, -1.0f, 1.0f);

    glUniformMatrix4fv(g->gui_projection_location, 1, GL_FALSE, projection.value);

    g->hotbar_sprite.position.y = -g->window_height / 2.0f / scale + g->hotbar_sprite.size.y / 2.0f;

    glEnable(GL_COLOR_LOGIC_OP);
    sprite_draw(&g->crosshair_sprite, g);
    glDisable(GL_COLOR_LOGIC_OP);

    sprite_draw(&g->hotbar_sprite, g);
}

void gui_destroy(gui *g)
{
    sprite_destroy(&g->crosshair_sprite);
    sprite_destroy(&g->hotbar_sprite);
    glDeleteTextures(1, &g->gui_texture);
    glDeleteProgram(g->gui_program);
}
