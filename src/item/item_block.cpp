struct Item_Block : public Item {
    Block *block;
    Texture *tex;

    Item_Block(itemid id, Block *_block) : Item(id), block(_block) {}

    void generate_block_textures(Frame_Buffer *fb, Vertex_Buffer *vbo, Index_Buffer *ibo) override {
        tex = xnew(Texture, 128, 128);
        fb->attach_texture(GL_COLOR_ATTACHMENT0, tex->id);

        GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};
        fb->attach_draw_buffers(1, draw_buffers);

        assert(fb->status() == GL_FRAMEBUFFER_COMPLETE);

        glViewport(0, 0, tex->width, tex->height);

        Static_Array<Chunk_Mesh::Vtx, 4 * 6> vs;
        Static_Array<u32, 6 * 6> is;
        
        Direction dirs[] = { UP, NORTH, EAST };
        for(u32 i = 0; i < array_length(dirs); i++) {
            auto dir = dirs[i];
            auto block_tex = block->get_texture(NULL, vec3i(), dir);

            u32 vis[4];
            for(u32 v = 0; v < 4; v++) {
                vis[v] = vs.push({
                    CUBE_VERTICES[CUBE_INDICES[dir][UNIQUE_INDICES[v]]] * 2.0f - vec3(1.0f, 1.0f, 1.0f),
                    CUBE_UVS[v],
                    block_tex,
                    FACE_SHADE[dir] + 0.05f
                });
            }

            for(u32 j = 0; j < 6; j++) {
                is.push(vis[CUBE_INDICES[0][j]]);
            }
        }

        vbo->set_subdata(&vs.data, vs.count * sizeof(Chunk_Mesh::Vtx));
        ibo->set_subdata(&is.data, is.count * sizeof(u32));

        glDrawElements(GL_TRIANGLES, is.count, GL_UNSIGNED_INT, 0);
    }

    Texture* get_texture(Item_Stack const& stack) override {
        return tex;
    }
};