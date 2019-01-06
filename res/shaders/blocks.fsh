#version 150 core

in vec2 pass_texCoord;

uniform sampler2D tex;

out vec4 out_color;

void main()
{
    out_color = texture(tex, pass_texCoord);
    if (out_color.a == 0.0)
        discard;
}