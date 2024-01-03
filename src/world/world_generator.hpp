struct World_Generator {
    World *world;

    World_Generator(World *_world) : world(_world) {}

    void deinit();

    void generate_chunk(Chunk *chunk);
};