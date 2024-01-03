#version 460 core

uniform mat4 u_proj;
uniform mat4 u_view;

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec4 v_color;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in int v_tex;

smooth out vec4 pass_color;
smooth out vec2 pass_uv;
flat out int pass_tex;

void main() {
    gl_Position = u_proj * u_view * vec4(v_pos.xyz, 1.0);
    pass_color = v_color;
    pass_uv = v_uv;
    pass_tex = v_tex;
}