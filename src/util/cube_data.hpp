vec3i const CUBE_MOORE_NEIGHBORHOOD[27] = {
    vec3i(-1,  1, -1), vec3i(0,  1, -1), vec3i(1,  1, -1),
    vec3i(-1,  0, -1), vec3i(0,  0, -1), vec3i(1,  0, -1),
    vec3i(-1, -1, -1), vec3i(0, -1, -1), vec3i(1, -1, -1),
    vec3i(-1,  1,  0), vec3i(0,  1,  0), vec3i(1,  1,  0),
    vec3i(-1,  0,  0), vec3i(0,  0,  0), vec3i(1,  0,  0),
    vec3i(-1, -1,  0), vec3i(0, -1,  0), vec3i(1, -1,  0),
    vec3i(-1,  1,  1), vec3i(0,  1,  1), vec3i(1,  1,  1),
    vec3i(-1,  0,  1), vec3i(0,  0,  1), vec3i(1,  0,  1),
    vec3i(-1, -1,  1), vec3i(0, -1,  1), vec3i(1, -1,  1)
};

vec2 const CUBE_UVS[4] = {
    vec2(1.0f, 0.0f),
    vec2(0.0f, 0.0f),
    vec2(0.0f, 1.0f),
    vec2(1.0f, 1.0f)
};

vec3 const CUBE_VERTICES[8] = {
    vec3(0, 0, 0), // 0
    vec3(1, 0, 0), // 1
    vec3(1, 1, 0), // 2
    vec3(0, 1, 0), // 3

    vec3(0, 0, 1), // 4
    vec3(1, 0, 1), // 5
    vec3(1, 1, 1), // 6
    vec3(0, 1, 1)  // 7
};

u32 const CUBE_INDICES[6][6] = {
    { 2, 3, 0, 0, 1, 2 }, // NORTH
    { 7, 6, 5, 5, 4, 7 }, // SOUTH
    { 6, 2, 1, 1, 5, 6 }, // EAST
    { 3, 7, 4, 4, 0, 3 }, // WEST
    { 6, 7, 3, 3, 2, 6 }, // UP
    { 0, 4, 5, 5, 1, 0 }  // DOWN
};

u32 const UNIQUE_INDICES[4] = { 1, 0, 4, 2 };

f32 const FACE_SHADE[6] = {
    0.90f, // NORTH
    0.65f, // SOUTH
    0.75f, // EAST
    0.75f, // WEST
    0.95f, // UP
    0.50f  // DOWN
};

vec3i const BLOCK_OFFSETS_SURROUNDING_VERTEX[8][8] = {
    { vec3i(0, 0, 0), vec3i(-1,  0,  0), vec3i(0, -1,  0), vec3i(-1, -1, 0), vec3i(0,  0, -1), vec3i(-1,  0, -1), vec3i(0, -1, -1), vec3i(-1, -1, -1) },
    { vec3i(0, 0, 0), vec3i( 1,  0,  0), vec3i(0, -1,  0), vec3i( 1, -1, 0), vec3i(0,  0, -1), vec3i( 1,  0, -1), vec3i(0, -1, -1), vec3i( 1, -1, -1) },
    { vec3i(0, 0, 0), vec3i( 1,  0,  0), vec3i(0,  1,  0), vec3i( 1,  1, 0), vec3i(0,  0, -1), vec3i( 1,  0, -1), vec3i(0,  1, -1), vec3i( 1,  1, -1) },
    { vec3i(0, 0, 0), vec3i(-1,  0,  0), vec3i(0,  1,  0), vec3i(-1,  1, 0), vec3i(0,  0, -1), vec3i(-1,  0, -1), vec3i(0,  1, -1), vec3i(-1,  1, -1) },
    { vec3i(0, 0, 0), vec3i(-1,  0,  0), vec3i(0, -1,  0), vec3i(-1, -1, 0), vec3i(0,  0,  1), vec3i(-1,  0,  1), vec3i(0, -1,  1), vec3i(-1, -1,  1) },
    { vec3i(0, 0, 0), vec3i( 1,  0,  0), vec3i(0, -1,  0), vec3i( 1, -1, 0), vec3i(0,  0,  1), vec3i( 1,  0,  1), vec3i(0, -1,  1), vec3i( 1, -1,  1) },
    { vec3i(0, 0, 0), vec3i( 1,  0,  0), vec3i(0,  1,  0), vec3i( 1,  1, 0), vec3i(0,  0,  1), vec3i( 1,  0,  1), vec3i(0,  1,  1), vec3i( 1,  1,  1) },
    { vec3i(0, 0, 0), vec3i(-1,  0,  0), vec3i(0,  1,  0), vec3i(-1,  1, 0), vec3i(0,  0,  1), vec3i(-1,  0,  1), vec3i(0,  1,  1), vec3i(-1,  1,  1) }
};

u32 const CUBE_LINE_INDICES[24] = {
    0, 1,
    0, 3,
    0, 4,
    1, 2,
    1, 5,
    2, 3,
    2, 6,
    3, 7,
    4, 5,
    4, 7,
    5, 6,
    6, 7
};