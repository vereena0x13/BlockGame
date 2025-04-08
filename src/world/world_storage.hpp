// TODO: i'm pretty sure this whole system could be a 
//       bit more efficient... we should avoid the polling 
//       that we're doing on chunks (regions, really) from World.

u32 const REGION_SIZE_LOG2 = 4;
u32 const REGION_SIZE = pow(REGION_SIZE_LOG2, 2);

struct Region_Save_Task : public Task {
    char path[1024];
    ByteBuf bb;
    vec3i region_pos;

    Region_Save_Task(vec3i _region_pos) : region_pos(_region_pos) {}

    void run() override;
    void deinit() override;
};

struct Region_Load_Task : public Task {
    struct World_Storage *storage;
    vec3i region_pos;

    Region_Load_Task(struct World_Storage *_storage, vec3i _region_pos) : storage(_storage), region_pos(_region_pos) {}

    void run() override;
};

struct World_Storage {
    struct Region {
        struct World *world;
        vec3i region_pos;

        vec3i min_chunk_pos;
        vec3i max_chunk_pos;

        bool dirty = false;
        Chunk* chunks[REGION_SIZE][REGION_SIZE][REGION_SIZE];

        Region(struct World *_world, vec3i _region_pos) : world(_world), region_pos(_region_pos) {
            min_chunk_pos = region_pos >> REGION_SIZE_LOG2;
            max_chunk_pos = min_chunk_pos + REGION_SIZE;
            memset(&chunks, 0, sizeof(chunks));
        }

        Chunk* get_chunk(vec3i pos) {
            return chunks[pos.x][pos.y][pos.z];
        }

        void set_chunk(vec3i pos, Chunk *chunk) {
            chunks[pos.x][pos.y][pos.z] = chunk;
        }

        void serialize(ByteBuf *bb);
        void deserialize(ByteBuf *bb);
    };

    struct World *world;
    str path;

    u32 write_timer = 0;

    Hash_Table<vec3i, Region*> regions;
    Hash_Table<vec3i, Region_Load_Task*> load_tasks;
    Array<Region*> load_queue;
    pthread_mutex_t load_queue_lock;

    World_Storage(World *_world, str name) : world(_world) {
        path = mkstr(NULL, strsz(name) + 6);
        sprintf(path, "saves/%s", name);
        strhdr(path)->size = strlen(path);

        regions.init();
        load_tasks.init();
        assert(pthread_mutex_init(&load_queue_lock, NULL) == 0);

        mkdir_if_not_exists(path);
        mkdir_if_not_exists(tsprintf("%s/regions", path));
    }

    void deinit() {
        xfree(strhdr(path));
        regions.free();
        load_tasks.free();
        load_queue.free();
        pthread_mutex_destroy(&load_queue_lock) == 0;
    }

    cstr get_file(cstr name);
    cstr get_file(rstr name);

    bool region_exists_on_disk(vec3i region_pos);
    bool has_region(vec3i region_pos);
    bool loading_region(vec3i region_pos);
    Region* get_region(vec3i region_pos);
    bool load_region(vec3i region_pos);

    Chunk* get_chunk(vec3i chunk_pos);
    void set_chunk(Chunk *chunk);
    void mark_dirty(vec3i chunk_pos);

    void update();
    void save_all_dirty(bool ignore_throttle = false);

private:
    void write_region(Region *region);
    void on_region_loaded(Region *region);

    friend Region_Save_Task;
    friend Region_Load_Task;
};