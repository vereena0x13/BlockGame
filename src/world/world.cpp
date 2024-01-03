World::World(str name) {
    storage = xnew(World_Storage, this, name);
    gen = xnew(World_Generator, this);
    chunks.init(4096);

    shader = xnew(Shader, "chunk", VERTEX_SHADER | FRAGMENT_SHADER);
    u_view = shader->get_uniform_location("u_view");
    u_proj = shader->get_uniform_location("u_proj");
    u_textures = shader->get_uniform_location("u_textures");
    u_view_pos = shader->get_uniform_location("u_view_pos");

    shader->set_uniform_1i(u_textures, 0);

    create_view_frustum();
}

void World::deinit() {
    // TODO

    for(u32 i = 0; i < chunks.size; i++) {
        auto const& slot = chunks.slots[i];
        if(slot.hash != 0) {
            auto const& chunk = slot.value;
            chunk->deinit();
            xfree(chunk);
        }
    }
    chunks.free();

    for(u32 i = 0; i < entities.count; i++) {
        auto e = entities[i];
        e->deinit();
        free(e);
    }
    entities.free();

    storage->deinit();
    xfree(storage);
    
    gen->deinit();
    xfree(gen);
}

#ifdef SHADER_HOT_RELOAD
void World::reload_chunk_shader() {
    if(shader->reload()) {
        u_view = shader->get_uniform_location("u_view");
        u_proj = shader->get_uniform_location("u_proj");
        u_textures = shader->get_uniform_location("u_textures");
        u_view_pos = shader->get_uniform_location("u_view_pos");

        shader->set_uniform_1i(u_textures, 0);
    } else {
        log(ERROR, "Failed to reload chunk shader");
    }
}
#endif

void World::add_entity(Entity *ent) {
    ent->world = this;
    entities.push(ent);
}

void World::set_player(Player *player) {
    this->player = player;
    add_entity(player);
}

void World::load() {
    TIMED_FUNCTION();

    auto player_path = storage->get_file("player.dat");
    ByteBuf bb;
    if(bb.read_from_file(player_path)) {
        player->deserialize(&bb);
        bb.deinit();
    }

    log(TRACE, "Loaded player data from '%s'", player_path);
}

void World::save_all() {
    TIMED_FUNCTION();

    save_player();
    storage->save_all_dirty(true);
}

void World::save_player() {
    TIMED_FUNCTION();

    auto path = storage->get_file("player.dat");
    ByteBuf bb;
    player->serialize(&bb);
    bb.write_to_file(path);
    bb.deinit();

    log(TRACE, "Saved player data to '%s'", path);
}

Chunk* World::get_chunk(vec3i chunk_pos) {
    TIMED_FUNCTION();

    auto chunk = chunks.get(chunk_pos);
    if(chunk) return chunk;

    bool load = false;
    bool loading = false;

    auto region_pos = chunk_pos >> REGION_SIZE_LOG2;
    if(storage->has_region(region_pos)) {
        chunk = storage->get_chunk(chunk_pos);
    } else if(storage->loading_region(region_pos) || storage->load_region(region_pos)) {
        return NULL;
    }

    if(!chunk) {
        chunk = xnew(Chunk, this, chunk_pos);
        gen->generate_chunk(chunk);
        storage->set_chunk(chunk);
    }
    
    if(chunk) chunks.set(chunk_pos, chunk);        
    return chunk;
}

Block* World::get_block(vec3i pos) {
    TIMED_FUNCTION();

    auto chunk_pos = pos >> CHUNK_SIZE_LOG2;
    auto chunk = get_chunk(chunk_pos);
    if(!chunk) return NULL;
    return BLOCKS[chunk->get_block(pos & (CHUNK_SIZE-1))];
}

void World::set_block(vec3i pos, blkid id) {
    TIMED_FUNCTION();

    auto chunk_pos = pos >> CHUNK_SIZE_LOG2;
    auto chunk = get_chunk(chunk_pos);
    assert(chunk); // TODO

    auto chunk_block_pos = pos & (CHUNK_SIZE-1);
    chunk->set_block(chunk_block_pos, id);
    storage->mark_dirty(chunk_pos);

    for(u32 i = 0; i < 27; i++) {
        vec3i offset = CUBE_MOORE_NEIGHBORHOOD[i];
        if(offset != vec3i()) {
            vec3i bp = pos + offset;
            vec3i bcp = bp >> CHUNK_SIZE_LOG2;
            if(bcp != chunk_pos) {
                get_chunk(bcp)->mesh->dirty = true;
            }
        }
    }
}

Block_Entity* World::get_block_entity(vec3i pos) {
    auto chunk_pos = pos >> CHUNK_SIZE_LOG2;
    auto chunk = get_chunk(chunk_pos);
    return chunk->get_block_entity(pos & (CHUNK_SIZE-1));
}

void World::update() {
    TIMED_FUNCTION();
    
    for(u32 i = 0; i < chunks.size; i++) {
        if(chunks.slots[i].hash != 0) {
            auto const& pos = chunks.slots[i].key;
            auto const& chunk = chunks.slots[i].value;
            assert(chunk);

            chunk->update();
        }
    }


    for(auto entity : entities) {
        entity->update();
    }


    storage->update();


    if(player && player_save_timer++ >= 60*60*10) {
        player_save_timer = 0;
        save_player();
    }


    time_ticks++;
}

s32 chunk_btf_cmp_fn(void const* a, void const* b) {
    auto ca = *cast(Chunk**, a);
    auto cb = *cast(Chunk**, b);
    if(ca->dist < cb->dist) return 1;
    if(ca->dist > cb->dist) return -1;
    return 0;
}

u32 World::draw_chunks(bool wireframe) {
    TIMED_FUNCTION();

    // NOTE: If the chunk shader (program) happens to be reloaded 
    // (only applicable in dev builds), and any of the shaders
    // fail to compile, or linking the shaders into the program fails,
    // we just bail. No chunks for you lmao.
    if(!shader->valid) return 0;

    // NOTE: Set up OpenGL, the view frustum, and shader.
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    
    view_frustum.configure(player->pos, player->pos + player->camera.dir(), vec3(0, 1, 0));

    shader->bind();
    shader->set_uniform_mat4(u_proj, game->perspective); // TODO
    shader->set_uniform_mat4(u_view, player->camera.view());
    shader->set_uniform_vec3(u_view_pos, player->pos);
    game->block_textures->bind(0);

    u32 opaques = 0;
    u32 transparents = 0;
    auto pp = player->pos;
    auto pc = floor(pp) >> CHUNK_SIZE_LOG2;
    s32 const N = 5; // NOTE TODO
    s32 const NN = ((N*2)+1)*((N*2)+1)*((N*2)+1); // TODO cleanup
    Static_Array<Chunk*, NN> to_draw; // back to front

    {
        TIMED_BLOCK("Cull");

        // NOTE: Pick out the chunks within our viewing distance that
        //   1. has a y coordinate >= 0
        //   2. exists (i.e. the Chunk is loaded)
        //   3. passes the frustum culling test
        for(s32 i = -N; i <= N; i++) {
            for(s32 j = -N; j <= N; j++) {
                for(s32 k = -N; k <= N; k++) {
                    auto p = pc + vec3i(i, j, k);
                    if(p.y < 0) continue;

                    auto c = get_chunk(p);
                    if(!c) continue;

                    auto pmin = p * CHUNK_SIZE;
                    auto pmax = pmin + CHUNK_SIZE;
                    auto bb = AABB(flt(pmin), flt(pmax));
                    if(view_frustum.test(bb) != Frustum::OUTSIDE) {
                        if(!c->prepare_draw()) continue;
                        c->dist = (pc - p).length_squared();
                        to_draw.push(c);
                    }
                }
            }
        }
    }

    {
        TIMED_BLOCK("Sort");
        
        // NOTE: Next, sort the chunks in back-to-front 
        // order in relation to the view position.
        qsort(to_draw.data, to_draw.count, sizeof(Chunk*), chunk_btf_cmp_fn);
    }

    {
        TIMED_BLOCK("Opaque");
    
        // NOTE: Draw the opaque parts of the chunk meshes first.
        // The order we draw the opaque parts in is irrelevant,
        // but I chose to draw it front-to-back. Theoretically
        // front-to-back can be faster in some cases but it's
        // basically negligible from the quick test I just did.
        for(s32 i = to_draw.count - 1; i >= 0; i--) {
            auto chunk = to_draw[i];
            if(chunk->draw(OPAQUE)) opaques++;        
        }
    }

    {
        TIMED_BLOCK("Transparent");

        // NOTE: Next, draw the transparent parts of the chunk meshes.
        // We must draw transparent geometry in order of furthest from
        // the camera to closest to the camera (back-to-front), hence
        // the qsort.
        for(auto chunk : to_draw) {
            if(chunk->draw(TRANSPARENT)) transparents++;        
        }
    }

    // NOTE: Aaaaand we're done; clean up or w/e.
    shader->unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    return max(opaques, transparents);
}