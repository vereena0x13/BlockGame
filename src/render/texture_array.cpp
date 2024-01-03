struct Texture_Array {
    GLuint id;
    u32 width;
    u32 height;
    u32 depth;
    u32 levels;
    s32 zindex = 0;
    Hash_Table<cstr, s32, cstr_hash_fn, cstr_eq_fn> file_name_to_index;

    Texture_Array(u32 _width, u32 _height, u32 _depth, u32 _levels = 1) : width(_width), height(_height), depth(_depth), levels(_levels) {
        assert(levels);

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id);
        glTextureStorage3D(id, levels, GL_RGBA8, width, height, depth);

        if(levels > 1) {
            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        file_name_to_index.init();
    }

    void deinit() {
        glDeleteTextures(1, &id);
        file_name_to_index.free();
    }

    void set_data(void *image, u32 depth, u32 level = 0) {
        u32 denom = (u32)powf(2, level);
        glTextureSubImage3D(id, level, 0, 0, depth, width / denom, height / denom, 1, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }

    void bind(u32 unit) {
        glBindTextureUnit(unit, id);
    }

    s32 alloc() {
        assert(zindex < depth);
        return zindex++;
    }

    s32 add_from_file(cstr name) {
        auto idx = file_name_to_index.index_of(name);
        if(idx != -1) {
            return file_name_to_index.slots[idx].value;
        }

        auto path = tsprintf("assets/textures/%s.png", name);

        str file = read_entire_file(path);
        if(!file) {
            log(WARN, "Unable to load texture '%s'", name);
            return -1;
        }

        s32 w, h, _n;
        u8 *image = stbi_load_from_memory((u8 const*)file, strsz(file), &w, &h, &_n, 4);
        if(!image) {
            xfree(strhdr(file));
            return -1;
        }
        xfree(strhdr(file));

        assert(w == width);
        assert(h == height);

        alpha_premultiply_in_place((u32*)image, w, h);

        s32 id = alloc();
        set_data(image, id, 0);
        for(u32 level = 1; level < levels; level++) {
            downsample_2x_in_place(image, w);
            w >>= 1;
            set_data(image, id, level);
        }

        stbi_image_free(image);

        file_name_to_index.set(name, id);

        log(TRACE, "Added texture '%s' to texture array %d (%d, %d) with %d MIP levels", name, this->id, width, height, levels);

        return id;
    }

    s32 add_from_file(rstr name) {
        return add_from_file(cast(cstr, name));
    }
};