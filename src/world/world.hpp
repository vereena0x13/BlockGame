struct World {
    Shader *shader;
    GLint u_view;
    GLint u_proj;
    GLint u_textures;
    GLint u_view_pos;

    World_Storage *storage;
    World_Generator *gen;

    Hash_Table<vec3i, Chunk*> chunks;
    Array<struct Entity*> entities;
    str path;

    Player *player;
    u32 player_save_timer = 0;

    u64 time_ticks = 0; // TODO: save this

    Frustum view_frustum;

    World(str name);
    void deinit();

    void create_view_frustum() {
        f32 aspect = cast(f32, game->window_width) / cast(f32, game->window_height);
        view_frustum = Frustum(game->fov, aspect, game->near, game->far);
    }

    #ifdef SHADER_HOT_RELOAD
    void reload_chunk_shader();
    #endif

    void add_entity(Entity *ent);
    void set_player(Player *player);

    void load();
    void save_all();
    void save_player();

    Chunk* get_chunk(vec3i chunk_pos);

    Block* get_block(vec3i pos);
    void set_block(vec3i pos, blkid id);
    Block_Entity* get_block_entity(vec3i pos);

    void update();
    u32 draw_chunks(bool wireframe = false);
};