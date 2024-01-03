#version 460 core

uniform sampler2D u_tex;

smooth in vec2 pass_uv;

layout(location = 0) out vec4 color;

void main() {
    color = texture(u_tex, pass_uv);
}