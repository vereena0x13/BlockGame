struct Block_Air : public Block {
    Block_Air(blkid id) : Block(id) {}

    bool should_draw_face(World *world, vec3i pos, Direction dir) override {
        return false;
    }

    bool is_solid() override {
        return false;
    }
};