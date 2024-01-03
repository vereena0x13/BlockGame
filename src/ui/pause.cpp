struct Pause : public Panel {
    Button *resume;
    Button *settings;
    Button *save_and_quit;

    Pause(Game *game) : Panel(game) {
        children.push(resume = xnew(Button, game, false, "Resume", vec2(400, 80)));
        children.push(settings = xnew(Button, game, false, "Settings", vec2(400, 80)));
        children.push(save_and_quit = xnew(Button, game, false, "Save and Quit", vec2(400, 80)));
    }

    void draw(Batch_Renderer *r) override {
        auto ws = vec2(game->window_width, game->window_height);
        auto hws = ws * 0.5f;

        resume->pos = hws - resume->size * 0.5f + vec2(0.0f, -150.0f);
        settings->pos = hws - settings->size * 0.5f;
        save_and_quit->pos = hws - save_and_quit->size * 0.5f + vec2(0.0f, 150.0f);

        r->push_solid_quad(0, 0, game->window_width, game->window_height, vec4(0, 0, 0, 0.85f));

        Panel::draw(r);
    }

    void update() override {
        if(resume->just_pressed) {
            game->set_gui(GUI_IN_GAME);
        } else if(settings->just_pressed) {
            game->push_gui();
            game->set_gui(GUI_SETTINGS);
        } else if(save_and_quit->just_pressed) {
            game->unload_world();
            game->set_gui(GUI_MAIN_MENU);
        }

        Panel::update();
    }

    void key_callback(s32 key, s32 scancode, s32 action, s32 mods) override {
        if(action == GLFW_RELEASE) {
            switch(key) {
                case GLFW_KEY_ESCAPE:
                    game->set_gui(GUI_IN_GAME);
                    break;
            }
        }
    }
};