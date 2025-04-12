u32 const CHUNK_SIZE_LOG2 = 4;
u32 const CHUNK_SIZE = pow(CHUNK_SIZE_LOG2, 2);

struct Chunk {
    struct World *world;

    vec3i chunk_pos;
    blkid blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    Hash_Table<vec3i, Block_Entity*> block_entities;

    volatile bool generated = false;
    struct Chunk_Mesh *mesh = NULL;

    f32 dist = 0;

    Chunk(World *_world, vec3i _chunk_pos);
    void deinit();

    void serialize(ByteBuf *bb);
    void deserialize(ByteBuf *bb);

    blkid get_block(vec3i pos) { return blocks[pos.x][pos.y][pos.z]; }
    void set_block(vec3i pos, blkid id);
    Block_Entity* get_block_entity(vec3i pos);

    bool prepare_draw();
    bool draw(bool transparent);

    void update();
};