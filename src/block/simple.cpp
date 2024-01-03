struct Block_Simple : public Block {
    rstr tex_name;
    s32 tex;

    Block_Simple(blkid id, rstr _tex_name) : Block(id), tex_name(_tex_name) {}

    void register_textures(Texture_Array *texs) override {
        tex = texs->add_from_file(tex_name);
    }

    s32 get_texture(World *world, vec3i pos, Direction face) override {
        return tex;
    }
};