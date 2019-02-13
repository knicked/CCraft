#version 150 core

in vec2 position;
in vec2 tex_coord;
in float tex_id;
in vec3 color;

uniform mat4 projection;
uniform mat4 model;

out vec2 pass_tex_coord;
out float pass_tex_id;
out vec3 pass_color;

void main()
{
    gl_Position = projection * model * vec4(position, 0.0, 1.0);
    pass_tex_coord = tex_coord;
    pass_tex_id = tex_id;
    pass_color = color;
}
