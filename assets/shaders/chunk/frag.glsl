#version 460 core

uniform sampler2DArray u_textures;
uniform vec3 view_pos;

smooth in vec3 pass_pos;
smooth in vec2 pass_uv;
flat in int pass_tex;
smooth in float pass_brightness;

layout(location = 0) out vec4 color;

const vec4 fog_color = vec4(139.0 / 255.0, 205.0 / 255.0, 206.0 / 255.0, 1.0);

void main() {
    vec4 c = texture(u_textures, vec3(pass_uv, pass_tex)) * 
        vec4(pass_brightness, pass_brightness, pass_brightness, 1.0);

    float dist = length(pass_pos - view_pos);
    float grad_start = 64.0;
    float grad_length = 10.0;
    float grad = clamp((dist - grad_start) / grad_length, 0.0, 1.0);

    color = mix(c, fog_color, grad);
}