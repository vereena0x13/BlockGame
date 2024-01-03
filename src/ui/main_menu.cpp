struct Main_Menu : public Panel {
    Button *new_world;
    Button *load_world;
    Button *settings;
    Button *quit;
    
    Main_Menu(Game *game) : Panel(game) {
        children.push(new_world = xnew(Button, game, false, "New World", vec2(400, 80)));
        children.push(load_world = xnew(Button, game, false, "Load World", vec2(400, 80)));
        children.push(settings = xnew(Button, game, false, "Settings", vec2(400, 80)));
        children.push(quit = xnew(Button, game, false, "Quit", vec2(400, 80)));
    }

    void draw(Batch_Renderer *r) override {
        auto ws = vec2(game->window_width, game->window_height);
        auto hws = ws * 0.5f;

        new_world->pos = hws - new_world->size * 0.5f + vec2(0.0f, -300.0f);
        load_world->pos = hws - load_world->size * 0.5f + vec2(0.0f, -150.0f);
        settings->pos = hws - settings->size * 0.5f;
        quit->pos = hws - quit->size * 0.5f + vec2(0.0f, 150.0f);
        
        Panel::draw(r);

        rstr s = "bŁOcc ģAÈm";
        f32 sw = game->font->width(s);
        draw_text(r, game->font, s, hws.x - sw * 0.5f, 250, vec4(1,1,1,1));
    }
    
    void update() override {
        if(new_world->just_pressed) {
            game->push_gui();
            game->set_gui(GUI_NEW_WORLD);
        } else if(load_world->just_pressed) {
            game->push_gui();
            game->set_gui(GUI_LOAD_WORLD);
        } else if(settings->just_pressed) {
            game->push_gui();
            game->set_gui(GUI_SETTINGS);
        } else if(quit->just_pressed) {
            game->quit = true;
        }
        
        Panel::update();
    }
};