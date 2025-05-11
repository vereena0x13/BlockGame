using blkid = newtype<u8, struct blkid_tag>;


struct Block {
    blkid id;

    Block(blkid _id) : id(_id) {}

    virtual bool is_solid() { return true; }

    virtual bool is_transparent() { return false; }

    virtual bool should_draw_face(World *world, vec3i pos, Direction dir);

    virtual void register_textures(Texture_Array *itexs) {}

    virtual s32 get_texture(World *world, vec3i pos, Direction face) { return -1; }

    virtual void get_aabbs(World *world, vec3i pos, Array<AABB>& aabbs) {
        if(is_solid()) {
            vec3 min = vec3(pos.x, pos.y, pos.z);
            vec3 max = vec3(pos.x + 1, pos.y + 1, pos.z + 1);
            aabbs.push({ min, max });
        }
    }

    virtual struct Block_Entity* create_block_entity(World *world, vec3i pos) { return NULL; }

    virtual void on_placed(World *world, vec3i pos, struct Hit *hit, f32 player_yaw) {}
};


#define BLOCKS(X) \
    X(air       ) \
    X(bedrock   ) \
    X(stone     ) \
    X(dirt      ) \
    X(grass     ) \
    X(sand      ) \
    X(glass     ) \
    X(iron_ore  ) \
    X(iron      ) \
    X(gold_ore  ) \
    X(gold      ) \
    X(furnace   )


extern Block* BLOCKS[1024];
extern blkid N_BLOCKS;


#define X(name) extern Block* block_##name;
BLOCKS(X)
#undef X


void register_blocks();