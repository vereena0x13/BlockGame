struct Panel : public Component {
    Array<Component*> children;

    Panel(Game *game) : Component(game) {}

    virtual void on_close() {
        
    }

    void deinit() override {
        for(auto child : children) {
            child->deinit();
            xfree(child);
        }
        children.free();
    }

    void reset() override {
        for(auto child : children) {
            child->reset();
        }
    }

    void draw(Batch_Renderer *r) override {
        for(auto child : children) {
            child->draw(r);
        }
    }

    void update() override {
        for(auto child : children) {
            child->update();
        }
    }

    void key_callback(s32 key, s32 scancode, s32 action, s32 mods) override {
        for(auto child : children) {
            child->key_callback(key, scancode, action, mods);
        }
    }

    void char_callback(u32 cp) override {
        for(auto child : children) {
            child->char_callback(cp);
        }
    }
};