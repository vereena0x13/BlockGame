bool ray_triangle_intersection(
    vec3 const& ro,            // ray origin
    vec3 const& rd,            // ray direction
    vec3 const& v0,            // triangle vertex 0
    vec3 const& v1,            // triangle vertex 1
    vec3 const& v2,            // triangle vertex 2
    f32& t
) {
    vec3 v1v0 = v1 - v0;
    vec3 v2v0 = v2 - v0;
    vec3 rov0 = ro - v0;
    vec3 n = cross(v1v0, v2v0);
    vec3 q = cross(rov0, rd);
    f32 d = 1.0 / dot(rd, n);
    f32 u = d * dot(-q, v2v0);
    f32 v = d * dot(q, v1v0);
    t = d * dot(-n, rov0);
    return !(u < 0.0 || u > 1.0 || v < 0.0 || (u + v) > 1.0);
}

bool ray_plane_intersection(
    vec3 const& n,             // plane normal
    vec3 const& p0,            // plane center point
    vec3 const& ro,            // ray origin
    vec3 const& rd,            // ray direction (normalized)
    f32& t                     // (out) intersection distance along ray
) { 
    f32 d = dot(n, rd);
    if (d > 1e-6) {
        vec3 v = p0 - ro;
        t = dot(v, n) / d;
        return t >= 0;
    }
    return false;
}

bool ray_aabb_intersection(
    AABB const& bb,            // AABB
    vec3 pos,                  // ray origin
    vec3 dr,                   // 1.0f / ray direction (normalized)
    f32& t                     // (out) intersection distance along ray
) {
    // ray/AABB intersection copied from stackexchange ;)
    vec3 t1 = (bb.min - pos) * dr;
    vec3 t2 = (bb.max - pos) * dr;

    vec3 pmin = min(t1, t2);
    vec3 pmax = max(t1, t2);
    f32 tmin = max(max(pmin.x, pmin.y), pmin.z);
    f32 tmax = min(min(pmax.x, pmax.y), pmax.z);

    // if tmax < 0, ray is intersecting AABB, but the whole AABB is behind us
    if(tmax < 0) return false;

    // if tmin > tmax, ray doesn't intersect AABB
    if(tmin > tmax) return false;

    t = tmin;
    return true;
}

struct Hit {
    bool hit;
    u32 iters;
    f32 dist;
    vec3 pos;
    vec3i block_pos;
    Direction face;
    u32 aabb_index;
    s32 axis;
    s32 axisu;
    s32 axisv;
    bool flipu;
    bool flipv;
    f32 fu;
    f32 fv;
    vec2 uv;
};

// NOTE TODO: So this works. As far as I can tell, at the time of writing this, the raycast
// performs correctly in all valid cases (i.e. player is not within a block). However, this is
// the opposite of good. We are doing _three different_ ray intersection tests! First we
// do the ray/AABB test, then the ray/triangle tests for the faces of the AABB, and then the
// ray/plane test. Why? Because I tried it and it randomly worked on the first try lmao.
// I was just trying to get ray/triangle working but uh, somehow by combining them all
// (in this order) it just magically worked. So.
// We _should_ be able to do this with basically just a single ray intersection function,
// I think. But uh, whatever for now. It works. Blegh.
//                                  - vereena, 5-15-21

Hit raycast(World *world, vec3 origin, vec3 dir, f32 max_dist) {
    TIMED_FUNCTION();

    f32 dist = 0.0f;
    vec3 pos = origin;
    Direction face = UNKNOWN;
    vec3i hit_bp;
    u32 aabb_index = 0;
    u32 iters = 0;
    bool hit = false;

    Array<AABB> bbs(temp_allocator);

    dir = normalize(dir);
    vec3 d = 1.0f / dir;

    while(dist < max_dist && ++iters < 150 && !hit) {
        vec3i block_pos = floor(pos);

        f32 fmin = FLT_MAX; // block face hit distance
        f32 h = 0.05f; // AABB hit distance
        hit = false;

        for(u32 i = 0; i < array_length(CUBE_MOORE_NEIGHBORHOOD); i++) {
            vec3i bp = block_pos + CUBE_MOORE_NEIGHBORHOOD[i];

            auto block = world->get_block(bp);
            if(!block || block == block_air) continue;

            bbs.clear();
            block->get_aabbs(world, bp, bbs);

            if(bbs.count == 0) continue;

            // calculate which AABB was intersected
            for(u32 j = 0; j < bbs.count; j++) {
                auto const& bb = bbs[j];

                f32 tbb = FLT_MAX;
                if(ray_aabb_intersection(bb, pos, d, tbb) && tbb < h) {
                    for(u32 k = 0; k < 6; k++) {
                        vec3 n = flt(DIRECTION_OFFSET[k]);
                        vec3 p = bb.center() + bb.half_size() * n; // face plane center point

                        vec3 vs[4];
                        for(u32 v = 0; v < 4; v++) {
                            vs[v] = bb.min + CUBE_VERTICES[CUBE_INDICES[k][UNIQUE_INDICES[v]]] * bb.size();
                        }

                        f32 t1 = FLT_MAX;
                        f32 t2 = FLT_MAX;
                        if((ray_triangle_intersection(pos, dir, vs[0], vs[1], vs[2], t1) && t1 < h) ||
                           (ray_triangle_intersection(pos, dir, vs[2], vs[3], vs[0], t2) && t2 < h)) {
                            f32 tf;
                            if(ray_plane_intersection(-n, p, pos, dir, tf) && tf < h) {   
                                h = min(t1, t2);
                                hit_bp = bp;
                                face = cast(Direction, k);
                                aabb_index = j;
                                hit = true;
                            }
                        }
                    }
                }
            }
        }

        // step forward along dir by h
        pos += dir * h;
        dist += h;
    }

    bbs.free();

    if(face == UNKNOWN) hit = false; // TODO

    // TODO: the UV calculation is totally wrong when we have negatives...

    s32 axis = -1;
    bool flipu = face == NORTH || face == EAST;
    bool flipv = face == DOWN;
    f32 fu = flipu ? -1 : 1;
    f32 fv = flipv ? -1 : 1;
    s32 axisu = -1;
    s32 axisv = -1;
    vec2 uv;
    if(hit) {
        axis = DIRECTION_AXIS[face];
        axisu = AXIS_OTHER_AXES[axis][0];
        axisv = AXIS_OTHER_AXES[axis][1];
        uv = vec2(
            (pos.extract_axis(axisu) - cast(f32, hit_bp.extract_axis(axisu))),
            (pos.extract_axis(axisv) - cast(f32, hit_bp.extract_axis(axisv)))
        );
        if(flipu) uv.x = 1 - uv.x;
        if(flipv) uv.y = 1 - uv.y;
    }

    return { hit, iters, dist, pos, hit_bp, face, aabb_index, axis, axisu, axisv, flipu, flipv, fu, fv, uv };
}