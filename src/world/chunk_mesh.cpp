Static_Array<Chunk_Mesh::Vtx, CHUNK_N_VERTICES> Chunk_Mesh::vertices;
Static_Array<u32, CHUNK_N_INDICES> Chunk_Mesh::indices;
Static_Array<Chunk_Mesh::Face, CHUNK_N_FACES> Chunk_Mesh::faces;

Chunk_Mesh::Chunk_Mesh(Chunk *_chunk) : chunk(_chunk) {
    world = chunk->world;

    vbo.init(sizeof(vertices.data), GL_DYNAMIC_DRAW);
    ibo.init(sizeof(indices.data), GL_DYNAMIC_DRAW);
    vao.init();
    Vtx::configure_vao(vao, vbo, ibo);
}

void Chunk_Mesh::deinit() {
    vao.deinit();
    vbo.deinit();
    ibo.deinit();
}

void Chunk_Mesh::render_face(Block *block, vec3i block_pos, Direction dir, AABB const& bb) {
    auto tex = block->get_texture(world, block_pos, dir);

    u32 is[4];
    for(u32 v = 0; v < 4; v++) {
        u32 vidx = CUBE_INDICES[dir][UNIQUE_INDICES[v]];
        vec3 p = CUBE_VERTICES[vidx] + vec3(block_pos.x, block_pos.y, block_pos.z);
        vec2 uv = CUBE_UVS[v];

        u32 bc = 0;
        for(u32 l = 0; l < 8; l++) {
            vec3i bp = block_pos + BLOCK_OFFSETS_SURROUNDING_VERTEX[vidx][l];
            auto block = world->get_block(bp);
            if(!block || block == block_air || block->is_transparent()) bc++;
        }

        f32 brightness = (FACE_SHADE[dir] + 1.0f - 1.0f / exp(bc * 0.3f) - 0.1f) * 0.5f;

        is[v] = vertices.push({ p, uv, tex, brightness });
    }

    u32 start_index = indices.count;
    for(u32 l = 0; l < 6; l++) {
        indices.push(is[CUBE_INDICES[0][l]]);
    }
    index_count += 6;

    if(block->is_transparent()) {
        vec3 n = flt(DIRECTION_OFFSET[dir]);
        vec3 p = bb.center() + bb.half_size() * n;
        faces.push({ p, 0, start_index });
    }
}

void Chunk_Mesh::render_block(Block *block, vec3i block_pos) {
    Array<AABB> bbs;
    block->get_aabbs(world, block_pos, bbs);
    assert(bbs.count == 1); // TODO

    AABB bb = bbs[0];

    for(u8 d = 0; d < 6; d++) {
        auto dir = cast(Direction, d);
        if(!block->should_draw_face(world, block_pos, dir)) continue;
        render_face(block, block_pos, dir, bb);
    }

    bbs.free();
}

void Chunk_Mesh::render() {    
    TIMED_FUNCTION();

    vertices.clear();
    indices.clear();
    faces.clear();
    index_count = 0;

    auto chunk_base_pos = chunk->chunk_pos * CHUNK_SIZE;

    for(s32 i = 0; i < CHUNK_SIZE; i++) {
        for(s32 j = 0; j < CHUNK_SIZE; j++) {
            for(s32 k = 0; k < CHUNK_SIZE; k++) {
                auto chunk_block_pos = vec3i(i, j, k);
                auto id = chunk->get_block(chunk_block_pos);
                if(id == 0) continue;

                auto block = BLOCKS[id];
                assert(block);

                auto block_pos = chunk_base_pos + chunk_block_pos;

                render_block(block, block_pos);
            }
        }
    }
    
    if(faces.count > 0) {
        sort(FULL, game->player->pos, true);
    } else {
        base_offset = 0;
        base_count = indices.count;
        trans_offset = base_count;
        trans_count = 0;
    }

    if(persist) {
        if(p_indices) xfree(p_indices);
        if(p_faces) xfree(p_faces);
        p_indices = cast(u32*, xalloc(sizeof(u32) * indices.count));
        p_faces = cast(Face*, xalloc(sizeof(Chunk_Mesh::Face) * faces.count));
        memcpy(p_indices, indices.data, sizeof(u32) * indices.count);
        memcpy(p_faces, faces.data, sizeof(Face) * faces.count);
    }

    vbo.set_subdata(vertices.data, vertices.count * sizeof(Chunk_Mesh::Vtx));
    ibo.set_subdata(indices.data, indices.count * sizeof(u32));
}

s32 chunk_face_depth_cmp(Chunk_Mesh::Face const* a, Chunk_Mesh::Face const* b) {
    return cast(s32, -sign(a->dist - b->dist));
}

void Chunk_Mesh::sort(Chunk_Mesh_Sort_Type type, vec3 view_pos, bool use_static_indices) {
    TIMED_FUNCTION();

    if(!use_static_indices) assert(persist);
    auto index_data = use_static_indices ? indices.data : p_indices;

    for(u32 i = 0; i < faces.count; i++) {
        auto& face = faces[i];
        face.dist = (view_pos - face.pos).length();
    }

    qsort(faces.data, faces.count, sizeof(Face), cast(int (*)(const void*, const void*), chunk_face_depth_cmp));

    u32 t_indices[CHUNK_N_INDICES];
    memcpy(t_indices, index_data, sizeof(u32) * index_count);

    Static_Bitset<CHUNK_N_FACES / 64> moved;

    for(u32 i = 0; i < faces.count; i++) {
        auto& face = faces[i];
        if(face.index != i * 6) {
            memcpy(
                &((u32*) index_data)[i * 6],
                &((u32*) t_indices)[face.index],
                sizeof(u32) * 6
            );
            moved.set(face.index / 6);
        }
        face.index = i * 6;
    }

    if(type == FULL) {
        trans_offset = 0;
        trans_count = faces.count * 6;
        base_offset = trans_count;
        base_count = 0;

        for(u32 i = 0; i < (indices.count / 6); i++) {
            if(!moved.get(i)) {
                memcpy(
                    &((u32*) index_data)[base_offset + base_count],
                    &((u32*) t_indices)[i * 6],
                    sizeof(u32) * 6
                );
                base_count += 6;
            }
        }
    }

    assert(base_count + trans_count == index_count);
}

bool Chunk_Mesh::prepare(vec3 view_pos) {
    if(dirty) {
        if(!throttles.chunk_mesh_gen.act()) return false;
        render();
        dirty = false;
        depth_sort = false;
    } else if(depth_sort) {
        if(persist && p_indices != NULL && p_faces != NULL) {
            sort(PARTIAL, view_pos, false);
            ibo.set_subdata(p_indices, base_count + trans_count);
        } else {
            render();
        }
        depth_sort = false;
    }
    return true;
}

bool Chunk_Mesh::draw(Chunk_Mesh_Part part) {
    u32 count, offset;
    if(part == OPAQUE) {
        count = base_count;
        offset = base_offset;
    } else {
        count = trans_count;
        offset = trans_offset;
    }

    if(count == 0) return false;

    vao.bind();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, cast(void*, offset * sizeof(u32)));
    vao.unbind();

    return true;
}

void Chunk_Mesh::set_persist(bool persist) {
    this->persist = persist;

    if(!persist) {
        if(p_indices) xfree(p_indices);
        if(p_faces) xfree(p_faces);
        p_indices = NULL;
        p_faces = NULL;
    }
}