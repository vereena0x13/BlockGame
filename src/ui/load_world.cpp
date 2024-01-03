struct Load_World : public Panel {
    struct Save {
        str name;
        Button *button;

        Save(str _name, Button *_button) : name(_name), button(_button) {}

        void deinit() {
            xfree(strhdr(name));
        }
    };

    Array<Save> saves;
    Button *back;

    Load_World(Game *game) : Panel(game) {
    }

    void deinit() override {
        Panel::deinit();
        for(auto& save : saves) {
            save.deinit();
        }
        saves.free();
    }

    void reset() override {
        Panel::reset();
        load_worlds();
    }

    void draw(Batch_Renderer *r) override {
        auto ws = vec2(game->window_width, game->window_height);
        auto hws = ws * 0.5f;

        f32 oy = -300.0f;
        for(u32 i = 0; i < saves.count; i++) {
            auto btn = saves[i].button;
            btn->pos = hws - btn->size * 0.5f + vec2(0.0f, oy);
            oy += 150.0f;
        }
        back->pos = hws - back->size * 0.5f + vec2(0.0f, oy);

        Panel::draw(r);
    }

    void update() override {
        if(back->just_pressed) {
            game->pop_gui();
        } else {
            for(u32 i = 0; i < saves.count; i++) {
                auto const& save = saves[i];
                if(save.button->just_pressed) {
                    game->load_world(save.name);
                    game->pop_gui();
                    game->set_gui(GUI_IN_GAME);
                    break;
                }
            }
        }

        Panel::update();
    }

    void load_worlds() {
        if(saves.count) {
            for(auto& save : saves) {
                save.deinit();
            }

            saves.clear();
            children.clear();
        }

        auto dir = opendir("saves");
        if(!dir) return;

        struct dirent *dp;
        while((dp = readdir(dir)) != NULL) {
            auto const& _name = dp->d_name;
            if(strcmp(_name, ".") == 0 || strcmp(_name, "..") == 0) continue;

            auto name = mkstr(cast(cstr, _name), strlen(_name));

            auto button = xnew(Button, game, false, name, vec2(300.0f, 80.0f));
            children.push(button);
            saves.push(Save(name, button));
        }

        closedir(dir);

        children.push(back = xnew(Button, game, false, "Back", vec2(300.0f, 80.0f)));
    }
};