struct Settings : public Panel {
    Button *back;

    Settings(Game *game) : Panel(game) {
        children.push(back = xnew(Button, game, false, "Back", vec2(400, 80)));
    }

    void draw(Batch_Renderer *r) override {
        auto ws = vec2(game->window_width, game->window_height);
        auto hws = ws * 0.5f;

        back->pos = hws - back->size * 0.5f + vec2(0.0f, 300.0f);
        
        Panel::draw(r);
    }

    void update() override {
        if(back->just_pressed) {
            game->pop_gui();
        }

        Panel::update();
    }
};