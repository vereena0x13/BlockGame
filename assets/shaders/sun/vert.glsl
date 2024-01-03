#version 460 core

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec2 v_uv;

smooth out vec2 pass_uv;

void main() {
    pass_uv = v_uv;
    gl_Position = u_proj * u_view * u_model * vec4(v_pos, 1.0);
}