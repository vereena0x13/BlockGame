struct Block_Glass : public Block {
    static constexpr Direction FACE_PLANE_VON_NEUMANN_NEIGHBORHOOD[6][4] = {
        { UP,    EAST,  DOWN,  WEST },  // NORTH
        { UP,    WEST,  DOWN,  EAST },  // SOUTH
        { UP,    NORTH, DOWN,  SOUTH }, // EAST
        { UP,    SOUTH, DOWN,  NORTH }, // WEST
        { NORTH, EAST,  SOUTH, WEST },  // UP
        { WEST,  SOUTH, EAST,  NORTH }  // DOWN
    };

    static constexpr u32 FACE_TEX[4][16] = {
        { 0, 1, 4, 9,  3, 5, 8, 14, 2, 10, 6, 13,  7, 12, 11, 15 }, // Z_AXIS
        { 0, 1, 2, 10, 3, 5, 7, 14, 4,  9, 6, 13,  8, 12, 11, 15 }, // X_AXIS
        { 0, 3, 4, 8,  1, 5, 9, 12, 2,  7, 6, 11, 10, 14, 13, 15 }, // UP
        { 0, 1, 2, 10, 3, 5, 7, 14, 4,  9, 6, 13,  8, 12, 11, 15 }  // DOWN
    };

    s32 texs[16];

    Block_Glass(blkid id) : Block(id) {
    }

    bool is_transparent() override { return true; }

    void register_textures(Texture_Array *itexs) override {
        for(u32 i = 0; i < array_length(texs); i++) {
            texs[i] = itexs->add_from_file(tsprintf("blocks/glass_%d", i));
        }
    }

    s32 get_texture(World *world, vec3i pos, Direction face) override {
        if(!world) return texs[0];
        
        u8 f = 0;
        for(u8 i = 0; i < 4; i++) {
            vec3i p = pos + DIRECTION_OFFSET[FACE_PLANE_VON_NEUMANN_NEIGHBORHOOD[face][i]];
            auto block = world->get_block(p);
            if(block == this) f |= 1 << i;
        }

        u32 i;
        switch(face) {
            case UP:
                i = 2;
                break;
            case DOWN:
                i = 3;
                break;
            default:
                i = DIRECTION_AXIS[face];
        }

        return texs[FACE_TEX[i][f]];
    }
};