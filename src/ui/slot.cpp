struct Slot : public Component {
    vec2 pos;
    vec2 size;

    Container *container;
    u32 index;

    bool hovered = false;
    bool pressed = false;
    bool last_pressed = false;
    bool just_pressed = false;

    Slot(Game *game, vec2 _size, Container *_container, u32 _index) : Component(game), size(_size), container(_container), index(_index) {
        assert(index < container->size);
    }

    void draw(Batch_Renderer *r) override {
        Component::draw(r);

        r->push_solid_quad(pos, size, vec4(1,0,0,1));

        auto stack = container->slots[index];
        if(stack.is_valid()) stack.draw(r, pos, size);

        if(pressed) r->push_solid_quad(pos, size, vec4(1,1,1,0.1f));
        else if(hovered) r->push_solid_quad(pos, size, vec4(1,1,1,0.25f));
    }

    void update() override {
        Component::update();
        
        auto mp = game->mouse_pos;
        hovered = between(mp, pos, pos + size);
        last_pressed = pressed;
        pressed = hovered && (game->mouse[LMB] || game->mouse[RMB]);
        just_pressed = pressed && !last_pressed;
    }
};