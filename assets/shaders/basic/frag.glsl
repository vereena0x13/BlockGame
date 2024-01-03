#version 460 core

smooth in vec4 pass_color;

layout(location = 0) out vec4 color;

void main() {
    color = pass_color;
}