struct Block_Grass : public Block {
    s32 tex_sides;
    s32 tex_top;
    s32 tex_bottom;

    Block_Grass(blkid id) : Block(id) {}

    void register_textures(Texture_Array *texs) override {
        tex_sides = texs->add_from_file("blocks/grass_side");
        tex_top = texs->add_from_file("blocks/grass");
        tex_bottom = texs->add_from_file("blocks/dirt");
    }

    s32 get_texture(World *world, vec3i pos, Direction face) override {
        if(face == UP) return tex_top;
        if(face == DOWN) return tex_bottom;
        return tex_sides;
    }
};