void World_Generator::deinit() {
    
}

void World_Generator::generate_chunk(Chunk *chunk) {
    TIMED_FUNCTION();

    auto const& chunk_pos = chunk->chunk_pos;
    if(chunk_pos.y < 5) {
        for(u32 i = 0; i < CHUNK_SIZE; i++) {
            for(u32 j = 0; j < CHUNK_SIZE; j++) {
                for(u32 k = 0; k < CHUNK_SIZE; k++) {
                    if(chunk_pos.y == 0 && j == 0) chunk->set_block(vec3i(i, j, k), block_bedrock->id);
                    else                           chunk->set_block(vec3i(i, j, k), block_stone->id);
                }
            }
        }
    } else if(chunk_pos.y == 5) {
        for(u32 i = 0; i < CHUNK_SIZE; i++) {
            for(u32 k = 0; k < CHUNK_SIZE; k++) {
                auto rh = noise2(
                    (cast(f32, chunk_pos.x * CHUNK_SIZE + i + 5000000) * 0.02f),
                    (cast(f32, chunk_pos.z * CHUNK_SIZE + k + 5000000) * 0.02f)
                ) * 0.5f + 0.5f;
                rh = clamp(rh, 0.0f, 1.0f);
                u32 h = cast(u32, floor(rh * 12.0f));
                for(u32 j = 0; j <= h; j++) {
                    auto id = block_dirt->id;
                    if(j == h) id = block_grass->id;
                    chunk->set_block(vec3i(i, j, k), id);
                }
            }   
        }
    }
}