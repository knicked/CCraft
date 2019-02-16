#version 150 core

in vec3 position;
in vec3 normal;
in vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float pass_brightness;
out vec2 pass_tex_coord;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    vec3 normal = normalize(mat3(transpose(inverse(model))) * normal);
    pass_brightness = 0.6;
    pass_brightness += clamp(dot(vec3(-0.5, 0.4, 0.0), normal), -0.15, 1.0) * 0.5;
    pass_brightness += clamp(dot(vec3(0.5, 0.4, 0.0), normal), -0.15, 1.0) * 0.5;
    pass_brightness = clamp(pass_brightness, 0.0, 1.0);
    pass_tex_coord = tex_coord;
}
