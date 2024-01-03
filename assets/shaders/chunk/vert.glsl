#version 460 core

uniform mat4 u_proj;
uniform mat4 u_view;

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in int v_tex;
layout(location = 3) in float v_brightness;

smooth out vec3 pass_pos;
smooth out vec2 pass_uv;
flat out int pass_tex;
smooth out float pass_brightness;

void main() {
    pass_pos = (u_view * vec4(v_pos, 1.0)).xyz;
    gl_Position = u_proj * u_view * vec4(v_pos, 1.0);
    pass_uv = v_uv;
    pass_tex = v_tex;
    pass_brightness = v_brightness;
}