struct World_Generator {
    World *world;

    World_Generator(World *_world) : world(_world) {}

    void deinit();

    void generate_chunk(Chunk *chunk);
};


struct Chunk_Generate_Task : public Task {
    World_Generator *gen;
    Chunk *chunk;

    Chunk_Generate_Task(World_Generator *_gen, Chunk *_chunk) : gen(_gen), chunk(_chunk) {}

    void run() override;
};