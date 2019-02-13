#ifndef GUI_H
#define GUI_H

#include "world.h"

typedef struct
{
    GLuint vao;
    GLuint vbo;
    vec2 position;
    vec2 size;
} gui_sprite;

typedef struct
{
    GLuint vao;
    GLuint vbo;
    GLsizei vert_count;
    vec2 position;
    char text[256];
} gui_text;

typedef struct
{
    float window_width;
    float window_height;
    int scale;

    world *w;

    shader gui_shader;

    GLuint widgets_texture;
    GLuint ascii_texture;

    gui_sprite crosshair_sprite;
    gui_sprite hotbar_sprite;
    gui_sprite hotbar_selection_sprite;

    gui_text texts[32];
    unsigned int num_texts;

    GLuint hotbar_item_vaos[256];
    GLuint hotbar_item_vbos[256];
} gui;

void gui_init(gui *g, world* w);
void gui_handle_input(gui *g, input* i);
void gui_draw(gui *g);
void gui_destroy(gui *g);

gui_text *gui_create_text(gui *g);
void gui_set_text(gui_text *text, const char *s);

#endif
