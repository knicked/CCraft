#version 150 core

in float pass_brightness;
in vec2 pass_texCoord;

uniform sampler2D tex;

out vec4 out_color;

void main()
{
    vec4 tex_color = texture(tex, pass_texCoord);
    if (tex_color.a == 0.0)
        discard;
    out_color = vec4(tex_color.xyz * pass_brightness, tex_color.a);
}