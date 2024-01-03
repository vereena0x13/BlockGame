struct Assets {
    Hash_Table<cstr, Texture*, cstr_hash_fn, cstr_eq_fn> textures;
    Texture *missing;

    Assets() {
        textures.init();
        missing = get_texture("missing");
    }

    void deinit() {
        for(u32 i = 0; i < textures.size; i++) {
            auto const& slot = textures.slots[i];
            if(slot.hash) {
                slot.value->deinit();
                xfree(slot.value);
            }
        }
        textures.free();
    }

    Texture* get_texture(cstr name, bool generate_mipmaps = false) {
        auto tex = textures.get(name);
        if(tex) return tex;

        tex = load_texture_from_file(name, generate_mipmaps);
        if(tex) {
            textures.set(name, tex);
            return tex;
        }

        return missing;
    }

    Texture* get_texture(rstr name, bool generate_mipmaps = false) {
        return get_texture((cstr)name, generate_mipmaps);
    }
};