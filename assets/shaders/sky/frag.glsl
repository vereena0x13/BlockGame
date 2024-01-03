#version 460 core

uniform mat4 u_view;
uniform vec3 u_view_pos;

//smooth in vec3 pass_pos;
smooth in vec3 pass_sky_dir;
//smooth in vec2 pass_uv;

layout(location = 0) out vec4 color;

#define pi 3.14159265359

float atan2(float y, float x) {
    return abs(sign(x)) * atan(y / x) + ((1 - sign(x)) / 2.0) * (1 + sign(y) - abs(sign(y))) * pi;
}

vec4 f(vec2 uv) {
    /*
    float rate = 1.0 / 8.0;
    float m = mod(uv.x, rate);
    float n = mod(uv.y, rate);
    m = float(m > 0.5 * rate);
    n = float(n > 0.5 * rate);
    return vec4(vec3(m * n), 1.0);
    */

    float s = sin(100 * uv.x) + sin(100 * uv.y);
    return s > 0 ? vec4(1) : vec4(0);

    //return texture(u_tex, vec2(u, v));
}

void main() {
    float phi = atan2(pass_sky_dir.z, pass_sky_dir.x);
    float theta = asin(pass_sky_dir.y);
    float u = 1 - (phi + pi) / (2 * pi);
    float v = (theta + pi / 2) / pi;
    //color = f(pass_uv);
    color = f(vec2(u, v));
    //color = vec4(pass_uv, 0, 1);
}