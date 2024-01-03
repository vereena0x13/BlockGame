Chunk::Chunk(World *_world, vec3i _chunk_pos) : 
    world(_world), 
    chunk_pos(_chunk_pos)
{
    memset(blocks, 0, sizeof(blocks));
    block_entities.init();
}

void Chunk::deinit() {
    block_entities.free();

    if(mesh) {
        mesh->deinit();
        xfree(mesh);
    }
}

void Chunk::serialize(ByteBuf *bb) {
    chunk_pos.write(bb);

    for(u32 x = 0; x < CHUNK_SIZE; x++) {
        for(u32 y = 0; y < CHUNK_SIZE; y++) {
            for(u32 z = 0; z < CHUNK_SIZE; z++) {
                bb->write_u32(blocks[x][y][z]);
            }
        }
    }

    bb->write_u16(block_entities.count);
    for(u32 i = 0; i < block_entities.size; i++) {
        if(block_entities.slots[i].hash != 0) {
            block_entities.slots[i].key.write(bb);
            block_entities.slots[i].value->serialize(bb);
        }
    }
}

void Chunk::deserialize(ByteBuf *bb) {
    chunk_pos = vec3i::read(bb);

    for(u32 x = 0; x < CHUNK_SIZE; x++) {
        for(u32 y = 0; y < CHUNK_SIZE; y++) {
            for(u32 z = 0; z < CHUNK_SIZE; z++) {
                blocks[x][y][z] = bb->read_u32();
            }
        }
    }

    u16 n = bb->read_u16();
    for(u16 i = 0; i < n; i++) {
        auto p = vec3i::read(bb);
        auto const& block = BLOCKS[blocks[p.x][p.y][p.z]];
        auto be = block->create_block_entity(world, chunk_pos * CHUNK_SIZE + p);
        assert(be != NULL);
        be->deserialize(bb);
        be->on_load(world);
        block_entities.set(p, be);
    }
}

void Chunk::set_block(vec3i pos, blkid id) {
    if(blocks[pos.x][pos.y][pos.z] == id) return;
    
    auto idx = block_entities.index_of(pos);
    if(idx != -1) {
        auto be = block_entities.slots[idx].value;
        be->deinit();
        xfree(be);
        assert(block_entities.remove(pos));
    }

    blocks[pos.x][pos.y][pos.z] = id;
    if(mesh) mesh->dirty = true;

    auto be = BLOCKS[id]->create_block_entity(world, pos);
    if(be) {
        block_entities.set(pos, be);
        be->on_load(world);
    }
}

Block_Entity* Chunk::get_block_entity(vec3i pos) {
    auto idx = block_entities.index_of(pos);
    if(idx == -1) return NULL;
    return block_entities.slots[idx].value;
}

bool Chunk::prepare_draw() {
    if(!mesh) mesh = xnew(Chunk_Mesh, this);
    return mesh->prepare(world->player->pos);
}

bool Chunk::draw(bool transparent) {
    assert(mesh);
    return mesh->draw(transparent ? TRANSPARENT : OPAQUE);
}

void Chunk::update() {
    TIMED_FUNCTION();
    
    for(u32 i = 0; i < block_entities.size; i++) {
        if(block_entities.slots[i].hash) {
            auto& be = block_entities.slots[i].value;
            be->update();
        }
    }


    if(mesh) {
        auto player = world->player;
        bool in_range = (player->chunk_pos - chunk_pos).length() < 4;
        mesh->depth_sort = (player->chunk_pos == chunk_pos && player->block_pos_changed) ||
                           (player->chunk_pos_changed && in_range);
        mesh->set_persist(in_range);
    }
}