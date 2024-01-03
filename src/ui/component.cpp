struct Component {
    Game *game;

    Component(Game *_game) : game(_game) {}

    virtual void deinit() {}

    virtual void reset() {}

    virtual void draw(Batch_Renderer *r) {}

    virtual void update() {}

    virtual void key_callback(s32 key, s32 scancode, s32 action, s32 mods) {}
    virtual void char_callback(u32 cp) {}
    virtual void scroll_callback(f64 x, f64 y) {}
    virtual void mouse_pos_callback(vec2 pos, vec2 delta) {}
};