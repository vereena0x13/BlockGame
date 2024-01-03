#version 460 core

uniform mat4 u_proj;
uniform mat4 u_view;

layout(location = 0) in vec4 v_pos;

//smooth out vec3 pass_pos;
//smooth out vec3 pass_sky_dir;
smooth out vec3 pass_sky_dir;
//smooth out vec2 pass_uv;

/*
vec2 sphere_map(in vec3 normal, in vec3 ecPosition3)
   {
   float m;
   vec3 r, u;
   u = normalize(ecPosition3);
   r = reflect(u, normal);
   m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z + 1.0) * (r.z + 1.0));
   return vec2 (r.x / m + 0.5, r.y / m + 0.5);
   }
*/

void main() {
    /*pass_pos = (u_view * vec4(v_pos, 1.0)).xyz;
    pass_sky_dir = normalize(pos);
    gl_Position = u_proj * u_view * vec4(pos, 1.0); */

    mat4 inverseProjection = inverse(u_proj);
    mat3 inverseModelview = transpose(mat3(u_view));
    vec3 unprojected = (inverseProjection * v_pos).xyz;
    pass_sky_dir = normalize(inverseModelview * unprojected);

    /*
    vec3 pos = (u_view * v_pos).xyz;
    mat3 mvi = transpose(mat3(u_view));
    vec3 n = mvi * vec3(0, 0, -1);
    pass_uv = sphere_map(n, pos);
    */

    gl_Position = v_pos;
}