void Region_Save_Task::run() {
    bb.write_to_file(path);
    log(TRACE, "Saved region (%d, %d, %d) to '%s'", region_pos.x, region_pos.y, region_pos.z, path);
}

void Region_Save_Task::deinit() {
    bb.deinit();
    Task::deinit();
}


void Region_Load_Task::run() {
    auto region = xnew(World_Storage::Region, storage->world, region_pos);

    char region_path[1024];
    sprintf(region_path, "%s/regions/region_%d_%d_%d.dat", storage->path, region_pos.x, region_pos.y, region_pos.z);
    
    ByteBuf bb;
    if(bb.read_from_file(region_path)) {
        region->deserialize(&bb);
        bb.deinit();
    }

    storage->on_region_loaded(region);
    log(TRACE, "Loaded region (%d, %d, %d) from '%s'", region_pos.x, region_pos.y, region_pos.z, region_path);
}


void World_Storage::Region::serialize(ByteBuf *bb) {
    for(u32 x = 0; x < REGION_SIZE; x++) {
        for(u32 y = 0; y < REGION_SIZE; y++) {
            for(u32 z = 0; z < REGION_SIZE; z++) {
                auto const& chunk = chunks[x][y][z];
                if(chunk) {
                    bb->write_u8(1);
                    chunk->serialize(bb);
                } else {
                    bb->write_u8(0);
                }
            }
        }
    }
}

void World_Storage::Region::deserialize(ByteBuf *bb) {
    for(u32 x = 0; x < REGION_SIZE; x++) {
        for(u32 y = 0; y < REGION_SIZE; y++) {
            for(u32 z = 0; z < REGION_SIZE; z++) {
                if(bb->read_u8()) {
                    auto pos = vec3i(x, y, z);
                    auto chunk = xnew(Chunk, world, pos);
                    chunk->deserialize(bb);
                    chunks[x][y][z] = chunk;
                }
            }
        }
    }
}


cstr World_Storage::get_file(cstr name) { return tsprintf("%s/%s", path, name); }
cstr World_Storage::get_file(rstr name) { return get_file(cast(cstr, name)); }

bool World_Storage::region_exists_on_disk(vec3i region_pos) {
    char region_path[1024];
    sprintf(region_path, "%s/regions/region_%d_%d_%d.dat", path, region_pos.x, region_pos.y, region_pos.z);
    return file_exists(region_path);
}

bool World_Storage::has_region(vec3i region_pos) { return regions.index_of(region_pos) != -1; }
bool World_Storage::loading_region(vec3i region_pos) { return load_tasks.index_of(region_pos) != -1; }

World_Storage::Region* World_Storage::get_region(vec3i region_pos) {
    s32 idx = regions.index_of(region_pos);
    if(idx == -1) return NULL;
    return regions.slots[idx].value;
}

bool World_Storage::load_region(vec3i region_pos) {
    if(!region_exists_on_disk(region_pos)) return false;
    if(loading_region(region_pos)) return false;

    auto task = xnew(Region_Load_Task, this, region_pos);
    load_tasks.set(region_pos, task);
    game->task_queue.enqueue(task);

    return true;
}

Chunk* World_Storage::get_chunk(vec3i chunk_pos) {
    auto region = get_region(chunk_pos >> REGION_SIZE_LOG2);
    return region->get_chunk(chunk_pos & REGION_SIZE-1);
}

void World_Storage::set_chunk(Chunk *chunk) {
    auto region = get_region(chunk->chunk_pos >> REGION_SIZE_LOG2);
    if(!region) {
        vec3i region_pos = chunk->chunk_pos >> REGION_SIZE_LOG2;
        region = xnew(Region, world, region_pos);
        regions.set(region_pos, region);
    }
    region->set_chunk(chunk->chunk_pos & REGION_SIZE-1, chunk);
    region->dirty = true;
}

void World_Storage::mark_dirty(vec3i chunk_pos) {
    auto region = get_region(chunk_pos >> REGION_SIZE_LOG2);
    region->dirty= true;
}

void World_Storage::update() {
    TIMED_FUNCTION();

    write_timer++;
    if(write_timer == 60*30) {
        write_timer = 0;
        save_all_dirty();
    }


    assert(pthread_mutex_lock(&load_queue_lock) == 0);
    for(u32 i = 0; i < load_queue.count; i++) {
        auto region = load_queue[i];
        auto const& region_pos = region->region_pos;
        assert(load_tasks.remove(region_pos));
        regions.set(region_pos, region);
    }
    load_queue.clear();
    assert(pthread_mutex_unlock(&load_queue_lock) == 0);
}

void World_Storage::save_all_dirty(bool ignore_throttle) {
    TIMED_FUNCTION();

    write_timer = 0;

    u32 written = 0;
    for(u32 i = 0; i < regions.size; i++) {
        if(regions.slots[i].hash != 0) {
            auto region = regions.slots[i].value;
            if(!region->dirty) continue;
            if(!ignore_throttle && !throttles.region_save.act()) break;
            write_region(region);
            written++;
        }
    }

    if(written) log(TRACE, "Saved %d regions to disk", written);
}

void World_Storage::write_region(Region *region) {
    TIMED_FUNCTION();

    if(!region->dirty) return;
    region->dirty = false;

    auto task = xnew(Region_Save_Task, region->region_pos);
    sprintf(task->path, "%s/regions/region_%d_%d_%d.dat", path, region->region_pos.x, region->region_pos.y, region->region_pos.z);
    region->serialize(&task->bb);
    game->task_queue.enqueue(task);
}

void World_Storage::on_region_loaded(Region *region) {
    assert(pthread_mutex_lock(&load_queue_lock) == 0);
    load_queue.push(region);
    assert(pthread_mutex_unlock(&load_queue_lock) == 0);
}