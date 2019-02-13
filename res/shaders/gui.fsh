#version 150 core

in vec2 pass_tex_coord;
in float pass_tex_id;
in vec3 pass_color;

uniform sampler2D gui_textures[2];

out vec4 out_color;

void main()
{
    vec4 texture_color = texture(gui_textures[int(pass_tex_id)], pass_tex_coord);
    out_color = vec4(texture_color.rgb * pass_color, texture_color.a);
    if (out_color.a == 0.0)
        discard;
}

