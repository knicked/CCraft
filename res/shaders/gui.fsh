#version 150 core

in vec2 pass_tex_coord;

uniform sampler2D gui_texture;

out vec4 out_color;

void main()
{
    out_color = texture(gui_texture, pass_tex_coord);
    if (out_color.a == 0.0)
        discard;
}

