#version 460 core

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec4 v_color;

smooth out vec4 pass_color;

void main() {
    pass_color = v_color;
    gl_Position = u_proj * u_view * u_model * vec4(v_pos, 1.0);
}