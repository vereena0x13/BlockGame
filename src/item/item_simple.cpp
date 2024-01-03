struct Item_Simple : public Item {
    Texture *tex;

    Item_Simple(itemid id, cstr name) : Item(id) {
        tex = game->assets->get_texture(tsprintf("items/%s", name));
    }

    Item_Simple(itemid id, rstr name) : Item_Simple(id, cast(cstr, name)) {}

    virtual Texture* get_texture(Item_Stack const& stack) override { return tex; }
};