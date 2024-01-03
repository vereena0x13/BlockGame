using itemid = u32;

struct Item {
    itemid id;

    Item(itemid _id) : id(_id) {}

    virtual void register_textures(Texture_Array *texs) {}
    
    virtual void generate_block_textures(Frame_Buffer *fb, Vertex_Buffer *vbo, Index_Buffer *ibo) {}

    virtual Texture* get_texture(struct Item_Stack const& stack) {
        return NULL;
    }
};


extern Item* ITEMS[1024];
extern itemid N_ITEMS;


struct Item_Stack {
    Item *item;
    u32 count;

    Item_Stack() : item(NULL), count(0) {}
    Item_Stack(Item *_item, u32 _count) : item(_item), count(_count) {}

    void serialize(ByteBuf *bb) const {
        bb->write_u32(item->id);
        bb->write_u32(count);
    }

    inline bool is_valid() const { return item != NULL && count > 0; }

    void draw(Batch_Renderer *r, vec2 pos, vec2 size) const {
        r->push_textured_quad(pos.x, pos.y, size.x, size.y, item->get_texture(*this));

        if(count > 1) {
            auto s = tsprintf("%d", count);
            auto sw = game->font->width(s);
            auto sh = game->font->height;
            draw_text(r, game->font, s, pos.x + size.x - sw, pos.y + size.y - sh, vec4(1));
        }
    }

    static Item_Stack deserialize(ByteBuf *bb) {
        u32 id = bb->read_u32();
        u32 count = bb->read_u32();
        return Item_Stack(ITEMS[id], count);
    }
};


#define ITEMS(X)      \
    X(bedrock       ) \
    X(stone         ) \
    X(dirt          ) \
    X(grass         ) \
    X(sand          ) \
    X(glass         ) \
    X(iron_ore      ) \
    X(iron_block    ) \
    X(iron_ingot    ) \
    X(gold_ore      ) \
    X(gold_block    ) \
    X(gold_ingot    ) \
    X(furnace       )

#define X(name) extern Item* item_##name;
ITEMS(X)
#undef X