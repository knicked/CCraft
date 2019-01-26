#version 150 core

in vec2 position;
in vec2 tex_coord;

uniform mat4 projection;
uniform mat4 model;

out vec2 pass_tex_coord;

void main()
{
    gl_Position = projection * model * vec4(position, 0.0, 1.0);
    pass_tex_coord = tex_coord;
}
