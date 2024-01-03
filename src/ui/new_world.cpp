struct New_World : public Panel {
    Text_Box *world_name;
    Button *create_world;
    Button *back;
    
    bool world_name_dirty = true;

    New_World(Game *game) : Panel(game) {
        children.push(world_name = xnew(Text_Box, game, vec2(650.0f, 80.0f)));
        children.push(create_world = xnew(Button, game, false, "Create World", vec2(300.0f, 80.0f)));
        children.push(back = xnew(Button, game, false, "Back", vec2(300.0f, 80.0f)));
    }

    virtual void reset() override {
        Panel::reset();
        world_name->chars.clear();
    }

    virtual void draw(Batch_Renderer *r) override {
        auto const& font = game->font;

        auto ws = vec2(game->window_width, game->window_height);
        auto hws = ws * 0.5f;

        world_name->pos = hws - world_name->size * 0.5f + vec2(0.0f, -150.0f);
        create_world->pos = hws - create_world->size * 0.5f + vec2(175.0f, 0.0f);
        back->pos = hws - back->size * 0.5f + vec2(-175.0f, 0.0f);

        draw_text(r, font, "World Name", world_name->pos.x, world_name->pos.y - font->height - 10.0f, vec4(1, 1, 1, 1));

        Panel::draw(r);
    }

    virtual void update() override {
        auto name = world_name->value();

        if(world_name_dirty) {
            world_name_dirty = false;
            create_world->enabled = strsz(name) > 0 && !game->world_exists(name);
        }

        if(create_world->just_pressed) {
            game->load_world(name);
            game->pop_gui();
            game->set_gui(GUI_IN_GAME);
        } else if(back->just_pressed) {
            game->pop_gui();
        }

        Panel::update();
    }

    void key_callback(s32 key, s32 scancode, s32 action, s32 mods) override {
        Panel::key_callback(key, scancode, action, mods);
        world_name_dirty = true;
    }

    void char_callback(u32 cp) override {
        Panel::char_callback(cp);
        world_name_dirty = true;
    }
};