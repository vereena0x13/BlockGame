/*
east: -90 / 270
west: 90 / -270
north: 0
south: 180
*/


#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif
#define M_PI2 (M_PI*2.0f)


#define deg2rad(x) (x*M_PI/180.0f)
#define rad2deg(x) (x*180.0f/M_PI)


#define Z_AXIS 0
#define X_AXIS 1
#define Y_AXIS 2


struct vec2 {
    f32 x;
    f32 y;

    constexpr vec2() : x(0), y(0) {}
    explicit constexpr vec2(f32 _x) : x(_x), y(_x) {}
    constexpr vec2(f32 _x, f32 _y) : x(_x), y(_y) {}

#ifndef IMGUI_DISABLE
    vec2(ImVec2 const& v) : x(v.x), y(v.y) {}
    operator ImVec2() const { return ImVec2(x, y); }
#endif

    f32 length_squared() { return x*x + y*y; }
    f32 length() { return sqrtf(length_squared()); }

    vec2& operator+=(vec2 const& b) {
        x += b.x;
        y += b.y;
        return *this;
    }

    vec2& operator-=(vec2 const& b) {
        x -= b.x;
        y -= b.y;
        return *this;
    }

    vec2 operator*=(vec2 const& b) {
        x *= b.x;
        y *= b.y;
        return *this;
    }

    bool operator==(vec2 const& b) const { return x == b.x && y == b.y; }
    bool operator!=(vec2 const& b) const { return !(*this == b); }

    void write(DataOutput *out) {
        out->write_f32(x);
        out->write_f32(y);
    }

    static vec2 read(DataInput *in) {
        f32 x = in->read_f32();
        f32 y = in->read_f32();
        return vec2(x, y);
    }
};

vec2 operator+(vec2 const& a, vec2 const& b) { return { a.x + b.x, a.y + b.y }; }
vec2 operator-(vec2 const& a, vec2 const& b) { return { a.x - b.x, a.y - b.y }; }
vec2 operator*(vec2 a, f32 b) { return { a.x * b, a.y * b }; }
vec2 operator*(f32 b, vec2 a) { return a * b; }
vec2 operator*(vec2 a, vec2 b) { return { a.x * b.x, a.y * b.y }; }

vec2 normalize(vec2 a) {
    f32 len = a.length();
    return vec2(a.x / len, a.y / len);
}

f32 distance(vec2 a, vec2 b) { return (a - b).length(); }
f32 dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
bool between(vec2 p, vec2 min, vec2 max) { return p.x >= min.x && p.y >= min.y && p.x <= max.x && p.y <= max.y; }


struct vec3 {
    f32 x;
    f32 y;
    f32 z;

    constexpr vec3() : x(0), y(0), z(0) {}
    explicit constexpr vec3(f32 _x) : x(_x), y(_x), z(_x) {}
    constexpr vec3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}

    f32 length_squared() { return x*x + y*y + z*z; }
    f32 length() { return sqrtf(length_squared()); }

    vec3& operator+=(vec3 const& b) {
        x += b.x;
        y += b.y;
        z += b.z;
        return *this;
    }

    vec3& operator-=(vec3 const& b) {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        return *this;
    }

    vec3 operator*=(vec3 const& b) {
        x *= b.x;
        y *= b.y;
        z *= b.z;
        return *this;
    }

    vec3 operator+(vec3 const& b) const { return { x + b.x, y + b.y, z + b.z }; }
    vec3 operator-(vec3 const& b) const { return { x - b.x, y - b.y, z - b.z }; }
    vec3 operator*(vec3 const& b) const { return { x * b.x, y * b.y, z * b.z }; }
    vec3 operator/(vec3 const& b) const { return { x / b.x, y / b.y, z / b.z }; }
    vec3 operator*(f32 b) const { return { x * b, y * b, z * b }; }
    vec3 operator/(f32 b) const { return { x / b, y / b, z / b }; }

    vec3 operator-() const { return { -x, -y, -z }; }

    bool operator==(vec3 const& b) const { return x == b.x && y == b.y && z == b.z; }
    bool operator!=(vec3 const& b) const { return !(*this == b); }

    f32 extract_axis(s32 axis) const {
        switch(axis) {
            case X_AXIS: return x;
            case Y_AXIS: return y;
            case Z_AXIS: return z;
        }
        assert(false);
    }

    void write(DataOutput *out) {
        out->write_f32(x);
        out->write_f32(y);
        out->write_f32(z);
    }

    static vec3 read(DataInput *in) {
        f32 x = in->read_f32();
        f32 y = in->read_f32();
        f32 z = in->read_f32();
        return vec3(x, y, z);
    }
};

vec3 operator*(f32 b, vec3 const& a) noexcept { return a * b; }
vec3 operator/(f32 a, vec3 const& b) noexcept { return { a / b.x, a / b.y, a / b.z }; }

vec3 normalize(vec3 a) noexcept {
    f32 len = a.length();
    return vec3(a.x / len, a.y / len, a.z / len);
}

f32 distance(vec3 const& a, vec3 const& b) noexcept { return (a - b).length(); }
f32 dot(vec3 const& a, vec3 const& b) noexcept { return a.x * b.x + a.y * b.y + a.z * b.z; }

vec3 cross(vec3 const& a, vec3 const& b) noexcept {
    return vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

vec3 eq(vec3 const& v, vec3 const& l) noexcept {
    return vec3(
        v.x == l.x ? 1 : 0,
        v.y == l.y ? 1 : 0,
        v.z == l.z ? 1 : 0
    );
}

vec3 lt(vec3 const& v, vec3 const& l) noexcept {
    return vec3(
        v.x < l.x ? 1 : 0,
        v.y < l.y ? 1 : 0,
        v.z < l.z ? 1 : 0
    );
}

vec3 gt(vec3 const& v, vec3 const& l) noexcept {
    return vec3(
        v.x > l.x ? 1 : 0,
        v.y > l.y ? 1 : 0,
        v.z > l.z ? 1 : 0
    );
}

constexpr vec3 min(vec3 a, vec3 b) noexcept { return vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
constexpr vec3 max(vec3 a, vec3 b) noexcept { return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }


struct vec4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;

    constexpr vec4() : x(0), y(0), z(0), w(0) {}
    explicit constexpr vec4(f32 _x) : x(_x), y(_x), z(_x), w(_x) {}
    constexpr vec4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}

#ifndef IMGUI_DISABLE
    vec4(ImVec4 const& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    operator ImVec4() const { return ImVec4(x, y, z, w); }
#endif
    
    f32 length_squared() { return x*x + y*y + z*z + w*w; }
    f32 length() { return sqrtf(length_squared()); }

    vec4& operator+=(vec4 const& b) {
        x += b.x;
        y += b.y;
        z += b.w;
        w += b.w;
        return *this;
    }

    vec4& operator-=(vec4 const& b) {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        w -= b.w;
        return *this;
    }

    vec4 operator+(vec4 const& b) const { return { x + b.x, y + b.y, z + b.z, w + b.w }; }
    vec4 operator-(vec4 const& b) const { return { x - b.x, y - b.y, z - b.z, w - b.w }; }
    vec4 operator*(vec4 const& b) const { return { x * b.x, y * b.y, z * b.z, w * b.w }; }
    vec4 operator/(vec4 const& b) const { return { x / b.x, y / b.y, z / b.z, w / b.w }; }
    vec4 operator*(f32 b) const { return { x * b, y * b, z * b, w * b }; }
    vec4 operator/(f32 b) const { return { x / b, y / b, z / b, w / b }; }

    bool operator==(vec4 const& b) const { return x == b.x && y == b.y && z == b.z && w == b.w; }
    bool operator!=(vec4 const& b) const { return !(*this == b); }

    void write(DataOutput *out) {
        out->write_f32(x);
        out->write_f32(y);
        out->write_f32(z);
        out->write_f32(w);
    }

    static vec4 read(DataInput *in) {
        f32 x = in->read_f32();
        f32 y = in->read_f32();
        f32 z = in->read_f32();
        f32 w = in->read_f32();
        return vec4(x, y, z, w);
    }
};

vec4 operator*(f32 b, vec4 const& a) { return a * b; }


struct mat4 {
    f32 m[4][4];

    mat4() {
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
        m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
    }

    mat4 operator*(mat4 const& b) {
        // NOTE TODO: We may want to optimize this eventually.
        mat4 r;
        memset(&r, 0, sizeof(mat4));

        for(s32 i = 0; i < 4; i++) {
            for(s32 j = 0; j < 4; j++) {
                for(s32 k = 0; k < 4; k++) {
                    r.m[i][j] += m[i][k] * b.m[k][j];
                }
            }    
        }

        return r;
    }

    vec4 operator*(vec4 const& r) {
        return vec4(
            m[0][0] * r.x + m[0][1] * r.y + m[0][2] * r.z + m[0][3] * r.w,
            m[1][0] * r.x + m[1][1] * r.y + m[1][2] * r.z + m[1][3] * r.w,
            m[2][0] * r.x + m[2][1] * r.y + m[2][2] * r.z + m[2][3] * r.w,
            m[3][0] * r.x + m[3][1] * r.y + m[3][2] * r.z + m[3][3] * r.w
        );
    }

    vec3 operator*(vec3 const& r) {
        return vec3(
            m[0][0] * r.x + m[0][1] * r.y + m[0][2] * r.z,
            m[1][0] * r.x + m[1][1] * r.y + m[1][2] * r.z,
            m[2][0] * r.x + m[2][1] * r.y + m[2][2] * r.z
        );
    }

    f32* value_ptr() {
        return &m[0][0];
    }

    static mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far) {
        f32 w = right - left;
        f32 h = top - bottom;
        f32 d = z_far - z_near;

        mat4 r = {};
        r.m[0][0] = 2.0f / w;
        r.m[1][1] = 2.0f / h; 
        r.m[2][2] = 2.0f / d;
        r.m[3][0] = -(right + left) / w;
        r.m[3][1] = -(top + bottom) / h;
        r.m[3][2] = -z_near / d;       
        return r;
    }

    static mat4 perspective(f32 fovy, f32 aspect, f32 zNear, f32 zFar) {
        f32 s = 1 / tan(deg2rad(fovy) * 0.5f);
        f32 dz = zFar - zNear;
        mat4 m;
        m.m[0][0] = s / aspect;
        m.m[1][1] = s;
        m.m[2][2] = -zFar / dz;
        m.m[3][2] = -zFar * zNear / dz;
        m.m[2][3] = -1.0f;
        m.m[3][3] = 0.0f;
        return m;
    }

    static mat4 translate(f32 x, f32 y, f32 z) {
        mat4 r = {};
        r.m[3][0] = x;
        r.m[3][1] = y;
        r.m[3][2] = z;
        return r;
    }

    static mat4 scale(f32 x, f32 y, f32 z) {
        mat4 r = {};
        r.m[0][0] = x;
        r.m[1][1] = y;
        r.m[2][2] = z;
        return r;
    }

    static mat4 rotate(f32 theta, vec3 axis) {
        f32 c = cosf(deg2rad(theta));
        f32 s = sinf(deg2rad(theta));
        f32 c1 = 1.0f - c;
        mat4 m;
        m.m[0][0] = axis.x * axis.x * c1 + c;
        m.m[0][1] = axis.x * axis.y * c1 + axis.z * s;
        m.m[0][2] = axis.x * axis.z * c1 - axis.y * s;
        m.m[0][3] = 0;
        m.m[1][0] = axis.x * axis.y * c1 - axis.z * s;
        m.m[1][1] = axis.y * axis.y * c1 + c;
        m.m[1][2] = axis.y * axis.z * c1 + axis.x * s;
        m.m[1][3] = 0;
        m.m[2][0] = axis.x * axis.z * c1 + axis.y * s;
        m.m[2][1] = axis.y * axis.z * c1 - axis.x * s;
        m.m[2][2] = axis.z * axis.z * c1 + c;
        m.m[2][3] = 0;
        m.m[3][0] = 0;
        m.m[3][1] = 0;
        m.m[3][2] = 0;
        m.m[3][3] = 1;
        return m;
    }
};


struct vec3i {
    s32 x;
    s32 y;
    s32 z;

    constexpr vec3i() : x(0), y(0), z(0) {}
    explicit constexpr vec3i(s32 _x) : x(_x), y(_x), z(_x) {}
    constexpr vec3i(s32 _x, s32 _y, s32 _z) : x(_x), y(_y), z(_z) {}

    f32 length_squared() { return x*x + y*y + z*z; }
    f32 length() { return sqrtf(length_squared()); }

    vec3i& operator+=(vec3 const& b) {
        x += b.x;
        y += b.y;
        z += b.z;
        return *this;
    }

    vec3i& operator-=(vec3 const& b) {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        return *this;
    }

    vec3i operator*=(vec3 const& b) {
        x *= b.x;
        y *= b.y;
        z *= b.z;
        return *this;
    }

    vec3i operator+(vec3i const& b) const { return vec3i(x + b.x,y + b.y,z + b.z); }
    vec3i operator-(vec3i const& b) const { return vec3i(x - b.x,y - b.y,z - b.z); }
    vec3i operator*(vec3i const& b) const { return vec3i(x * b.x,y * b.y,z * b.z); }
    vec3i operator/(vec3i const& b) const { return vec3i(x / b.x,y / b.y,z / b.z); }

    vec3i operator+(s32 a) const { return vec3i(x + a, y + a, z + a); }
    vec3i operator-(s32 a) const { return vec3i(x - a, y - a, z - a); }
    vec3i operator*(s32 a) const { return vec3i(x * a, y * a, z * a); }
    vec3i operator/(s32 a) const { return vec3i(x / a, y / a, z / a); }
    vec3i operator&(s32 a) const { return vec3i(x & a, y & a, z & a); }
    vec3i operator|(s32 a) const { return vec3i(x | a, y | a, z | a); }
    vec3i operator<<(s32 a) const { return vec3i(x << a, y << a, z << a); }
    vec3i operator>>(s32 a) const { return vec3i(x >> a, y >> a, z >> a); }

    bool operator==(vec3i const& b) const { return x == b.x && y == b.y && z == b.z; }
    bool operator!=(vec3i const& b) const { return !(*this == b); }

    s32 extract_axis(s32 axis) const {
        switch(axis) {
            case X_AXIS: return x;
            case Y_AXIS: return y;
            case Z_AXIS: return z;
        }
        assert(false);
    }

    void write(DataOutput *out) {
        out->write_s32(x);
        out->write_s32(y);
        out->write_s32(z);
    }

    static vec3i read(DataInput *in) {
        f32 x = in->read_s32();
        f32 y = in->read_s32();
        f32 z = in->read_s32();
        return vec3i(x, y, z);
    }
};

vec3i floor(vec3 x) { return vec3i(floor(x.x), floor(x.y), floor(x.z)); }
vec3i ceil(vec3 x) { return vec3i(ceil(x.x), ceil(x.y), ceil(x.z)); }
vec3i abs(vec3i x) { return vec3i(abs(x.x), abs(x.y), abs(x.z)); }
vec3 flt(vec3i x) { return vec3(x.x, x.y, x.z); }

vec3 operator+(vec3 const& a, vec3i const& b) { return vec3(cast(f32, b.x) + a.x, cast(f32, b.y) + a.y, cast(f32, b.z) + a.z); }
vec3 operator+(vec3i const& a, vec3 const& b) { return vec3(cast(f32, a.x) + b.x, cast(f32, a.y) + b.y, cast(f32, a.z) + b.z); }


#define DIRECTIONS(X)                             \
    X(NORTH,    0,   0,  0, -1,  SOUTH,   Z_AXIS) \
    X(SOUTH,    1,   0,  0,  1,  NORTH,   Z_AXIS) \
    X(EAST,     2,   1,  0,  0,  WEST,    X_AXIS) \
    X(WEST,     3,  -1,  0,  0,  EAST,    X_AXIS) \
    X(UP,       4,   0,  1,  0,  DOWN,    Y_AXIS) \
    X(DOWN,     5,   0, -1,  0,  UP,      Y_AXIS) \
    X(UNKNOWN,  6,   0,  0,  0,  UNKNOWN, -1    )

#define X(name, id, ox, oy, oz, opposite, axis) name = id,
enum Direction : u8 {
    DIRECTIONS(X)
};
#undef X

#define X(name, id, ox, oy, oz, opposite, axis) #name,
rstr const DIRECTION_NAME[7] = {
    DIRECTIONS(X)
};
#undef X

#define X(name, id, ox, oy, oz, opposite, axis) vec3i(ox, oy, oz),
vec3i const DIRECTION_OFFSET[7] = {
    DIRECTIONS(X)
};
#undef X

#define X(name, id, ox, oy, oz, opposite, axis) opposite,
Direction const DIRECTION_OPPOSITE[7] = {
    DIRECTIONS(X)
};
#undef X

#define X(name, id, ox, oy, oz, opposite, axis) axis,
s32 const DIRECTION_AXIS[7] = {
    DIRECTIONS(X)
};
#undef X

s32 const AXIS_OTHER_AXES[3][2] = {
    { X_AXIS, Y_AXIS },
    { Z_AXIS, Y_AXIS },
    { Z_AXIS, X_AXIS }
};

vec3 const AXIS_VEC3[3] = {
    vec3(0, 0, 1),
    vec3(1, 0, 0),
    vec3(0, 1, 0)
};

Direction angle_to_direction(f32 angle) {
    if(angle < 45 && angle >= 0 || angle <= 360 && angle >= 315) {
        return NORTH;
    } else if(angle >= 225 && angle < 315) {
        return EAST;
    } else if(angle >= 135 && angle< 225) {
        return SOUTH;
    } else {
        return WEST;
    }
}


struct quat {
    f32 r;
    union {
        struct {
            f32 i;
            f32 j;
            f32 k;
        };
        vec3 ijk;
    };

    quat() : r(0), i(0), j(0), k(0) {}
    quat(f32 _r, f32 _i, f32 _j, f32 _k) : r(_r), i(_i), j(_j), k(_k) {}
    quat(f32 _r, vec3 _ijk) : quat(_r, _ijk.x, _ijk.y, _ijk.z) {}
    quat(vec3 _ijk) : quat(0, _ijk) {}

    quat operator+(quat const& b) const { return quat(r + b.r, i + b.i, j + b.j, k + b.k); }
    quat operator+(f32 s) const { return quat(r + s, ijk); }
    quat operator*(f32 s) const { return quat(r * s, ijk * s); }
    quat operator/(f32 s) const { return quat(r / s, ijk / s); }
    
    quat operator*(quat const& b) const {
        return quat(
            r*b.r - i*b.i - j*b.j - k*b.k,
            r*b.i + i*b.r + j*b.k - k*b.j,
            r*b.j - i*b.k + j*b.r + k*b.i,
            r*b.k + i*b.j - j*b.i + k*b.r
        );
    }

    vec3 axis() { return ijk / sqrtf(i*i + j*j + k*k); }
    f32 angle() { return 2 * atan2f(sqrtf(i*i + j*j + k*k), r); }

    mat4 matrix();

    static quat rotate(f32 theta, vec3 axis) {
        f32 ht = deg2rad(theta) * 0.5f;
        return quat(cosf(ht), normalize(axis) * sinf(ht));
    }
};

quat conj(quat const& q) { return quat(q.r, -q.ijk); }
f32 normsq(quat const& q) { return q.r*q.r + q.i*q.i + q.j*q.j + q.k*q.k; }
f32 norm(quat const& q) { return sqrtf(normsq(q)); }
quat inv(quat const& q) { return conj(q) / normsq(q); }
quat conj(quat const& q, quat const& p) { return q * p * inv(q); }
vec3 conj(quat const& q, vec3 const& v) { return conj(q, quat(v)).ijk; }
quat unit(quat const& q) { return q / norm(q); }

mat4 quat::matrix() {
    f32 s = powf(norm(*this), -2);
    mat4 m;
    m.m[0][0] = 1 - 2 * s * (powf(j, 2) + powf(k, 2));
    m.m[0][1] = 2 * s * (i * j - k * r);
    m.m[0][2] = 2 * s * (i * k + j * r);
    m.m[1][0] = 2 * s * (i * j + k * r);
    m.m[1][1] = 1 - 2 * s * (powf(i, 2) + powf(k, 2));
    m.m[1][2] = 2 * s * (j * k - i * r);
    m.m[2][0] = 2 * s * (i * k - j * r);
    m.m[2][1] = 2 * s * (j * k + i * r);
    m.m[2][2] = 1 - 2 * s * (powf(i, 2) + powf(j, 2));
    return m;
}


struct AABB {
    struct Hit {
        bool hit;
        f32 h;
        vec3 n;
    };

    vec3 min;
    vec3 max;

    AABB() {}
    AABB(vec3 _min, vec3 _max) : min(_min), max(_max) {}

    bool intersects(AABB const& b) const {
        return !(
            (b.max.x <= min.x) ||
            (b.min.x >= max.x) ||
            (b.max.y <= min.y) ||
            (b.min.y >= max.y) ||
            (b.max.z <= min.z) ||
            (b.min.z >= max.z)
        );
    }

    void write(DataOutput *out) {
        min.write(out);
        max.write(out);
    }

    static AABB read(DataInput *in) {
        auto min = vec3::read(in);
        auto max = vec3::read(in);
        return AABB(min, max);
    }

    vec3 center() const { return (min + max) * 0.5f; }
    vec3 size() const { return max - min; }
    vec3 half_size() const { return size() * 0.5f; }
    static AABB from_center(vec3 const& center, vec3 const& half_size) { return { center - half_size, center + half_size }; }
    static AABB disjunction(AABB const& a, AABB const& b) { return { ::min(a.min, b.min), ::max(a.max, b.max) }; }

    static Hit sweep(AABB const& a, AABB const& b, vec3 const& delta) {
        vec3 dg0 = gt(delta, vec3());
        vec3 dl0 = vec3(1) - dg0;

        vec3 l0 = b.min * dg0 + b.max * dl0;
        vec3 r0 = a.max * dg0 + a.min * dl0;
        vec3 l1 = b.max * dg0 + b.min * dl0;
        vec3 r1 = a.min * dg0 + a.max * dl0;
  
        vec3 inv_entry = l0 - r0;
        vec3 inv_exit = l1 - r1;

        vec3 entry;
        vec3 exit;

        #define F(axis) \
            if(delta.axis == 0) { \
                entry.axis = -FLT_MAX; \
                exit.axis = FLT_MAX; \
            } else { \
                entry.axis = inv_entry.axis / delta.axis; \
                exit.axis = inv_exit.axis / delta.axis; \
            }
        F(x)
        F(y)
        F(z)
        #undef X

        f32 max_entry = ::max(::max(entry.x, entry.y), entry.z);
        f32 min_exit = ::min(::min(exit.x, exit.y), exit.z);

        if((max_entry > min_exit) || ((entry.x < 0.0f) && (entry.y < 0.0f) && (entry.z < 0.0f)) || (entry.x > 1.0f) || (entry.y > 1.0f) || (entry.z > 1.0f)) {
            return { false, 1.0f };
        }

        assert(max_entry >= 0.0f && max_entry <= 1.0f);

        vec3 n;
        if(entry.x > entry.y && entry.x > entry.z) {
            n = vec3(inv_entry.x > 0 ? -1 : 1, 0, 0);
        } else if(entry.y > entry.x && entry.y > entry.z) {
            n = vec3(0, inv_entry.y > 0 ? -1 : 1, 0);
        } else if(entry.z > entry.x && entry.z > entry.y) {
            n = vec3(0, 0, inv_entry.z > 0 ? -1 : 1);
        } else {
            return { false, 1.0f };
        }

        return { true, max_entry, n };
    }
};


struct Plane {
    vec3 p;
    vec3 n;
    f32 d;

    Plane() {}

    Plane(vec3 const& v0, vec3 const& v1, vec3 const& v2) {
        vec3 a = v0 - v1;
        vec3 b = v2 - v1;
        n = normalize(cross(a, b));
        p = v1;
        d = -dot(n, p);
    }

    f32 dist(vec3 const& p) const { return d + dot(n, p); }
};


struct Frustum {
    enum {
        TOP    = 0,
        BOTTOM = 1,
        LEFT   = 2,
        RIGHT  = 3,
        NEAR   = 4,
        FAR    = 5
    };

    enum {
        INSIDE    = 0,
        OUTSIDE   = 1,
        INTERSECT = 2
    };

    Plane planes[6];
    vec3 ntl, ntr, nbl, nbr,
         ftl, ftr, fbl, fbr;
    f32 near, far, angle, ratio, tan;
    f32 nw, nh, fw, fh;

    Frustum() {}

    Frustum(f32 fovy, f32 aspect, f32 _near, f32 _far) : angle(fovy), ratio(aspect), near(_near), far(_far) {
        tan = tanf(deg2rad(angle) * 0.5f);
        nh = near * tan;
        nw = nh * ratio;
        fh = far * tan;
        fw = fh * ratio;
    }

    void configure(vec3 const& p, vec3 const& l, vec3 const& u) {
        vec3 z = normalize(p - l);
        vec3 x = normalize(cross(u, z));
        vec3 y = cross(x, z);

        vec3 nc = p - z * near;
        vec3 fc = p - z * far;

        ntl = nc + y * nh - x * nw;
        ntr = nc + y * nh + x * nw;
        nbl = nc - y * nh - x * nw;
        nbr = nc - y * nh + x * nw;

        ftl = fc + y * fh - x * fw;
        ftr = fc + y * fh + x * fw;
        fbl = fc - y * fh - x * fw;
        fbr = fc - y * fh + x * fw;

        planes[TOP]    = Plane(ntr, ntl, ftl);
        planes[BOTTOM] = Plane(nbl, nbr, fbr);
        planes[LEFT]   = Plane(ntl, nbl, fbl);
        planes[RIGHT]  = Plane(nbr, ntr, fbr);
        planes[NEAR]   = Plane(ntl, ntr, nbr);
        planes[FAR]    = Plane(ftr, ftl, fbl);
    }

    s32 test(vec3 const& p) const {
        for(u32 i = 0; i < 6; i++) {
            if(planes[i].dist(p) < 0)
                return OUTSIDE;
        }
        return INSIDE;
    }

    s32 test(AABB const& bb) const {
        s32 r = INSIDE;
        for(u32 i = 0; i < 6; i++) {
            auto const& pl = planes[i];
            if(pl.dist(aabb_max_vertex(bb, pl.n)) < 0)
                return OUTSIDE;
            else if(pl.dist(aabb_min_vertex(bb, pl.n)) < 0)
                r = INTERSECT;
        }
        return r;
    }

private:
    static vec3 aabb_min_vertex(AABB const& bb, vec3 const& n) {
        vec3 r = bb.max;
        if(n.x >= 0) r.x = bb.min.x;
        if(n.y >= 0) r.y = bb.min.y;
        if(n.z >= 0) r.z = bb.min.z;
        return r;
    }

    static vec3 aabb_max_vertex(AABB const& bb, vec3 const& n) {
        vec3 r = bb.min;
        if(n.x >= 0) r.x = bb.max.x;
        if(n.y >= 0) r.y = bb.max.y;
        if(n.z >= 0) r.z = bb.max.z;
        return r;
    }
};


vec4 rgba255_to_rgba1(u32 c) {
    u8 r = c & 0xFF;
    u8 g = (c >> 8) & 0xFF;
    u8 b = (c >> 16) & 0xFF;
    u8 a = (c >> 24) & 0xFF;
    return {
        (f32)r / 255.0f,
        (f32)g / 255.0f,
        (f32)b / 255.0f,
        (f32)a / 255.0f
    };
}

u32 rgba1_to_rgba255(vec4 c) {
    u8 r = (u8) roundf(c.x * 255.0f);
    u8 g = (u8) roundf(c.y * 255.0f);
    u8 b = (u8) roundf(c.z * 255.0f);
    u8 a = (u8) roundf(c.w * 255.0f);
    return a << 24 | b << 16 | g << 8 | r;
}

vec4 rgba1_to_linear(vec4 c) {
    return {
        square(c.x),
        square(c.y),
        square(c.z),
        c.w
    };
}

vec4 linear_to_rgba1(vec4 c) {
    return {
        sqrtf(c.x),
        sqrtf(c.y),
        sqrtf(c.z),
        c.w
    };
}

vec4 alpha_premultiply(vec4 c) {
    auto p = rgba1_to_linear(c);
    p.x *= p.w;
    p.y *= p.w;
    p.z *= p.w;
    return linear_to_rgba1(p);
}