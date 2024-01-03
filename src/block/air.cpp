struct Block_Air : public Block {
    Block_Air(u32 id) : Block(id) {}

    bool should_draw_face(World *world, vec3i pos, Direction dir) override {
        return false;
    }

    bool is_solid() override {
        return false;
    }
};