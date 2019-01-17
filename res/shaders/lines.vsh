#version 150 core

in vec3 position;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(position, 1.0);
    gl_Position.z -= 0.0003;
}