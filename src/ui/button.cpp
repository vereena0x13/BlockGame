struct Button : public Component {
    cstr text;
    vec2 pos;
    vec2 size;

    Texture *tex = NULL;
    Texture *tex_hovered = NULL;
    Texture *tex_pressed = NULL;

    vec4 text_color = vec4(1, 1, 1, 1);
    vec4 text_color_hovered = vec4(1, 1, 1, 1);
    vec4 text_color_pressed = vec4(1, 1, 1, 1);

    bool enabled = true;
    bool hovered = false;
    bool pressed = false;
    bool last_pressed = false;
    bool just_pressed = false;

    Button(Game *game, bool alt, cstr _text, vec2 _pos, vec2 _size) : Component(game), text(_text), pos(_pos), size(_size) {
        auto const& assets = game->assets;
        if(alt) {
            tex = assets->get_texture("ui/button_alt/base");
            tex_pressed = assets->get_texture("ui/button_alt/pressed");
        } else {
            tex = assets->get_texture("ui/button/base");
            tex_pressed = assets->get_texture("ui/button/pressed");
        }
    }

    Button(Game *game, bool alt, rstr text, vec2 pos, vec2 size) : Button(game, alt, cast(cstr, text), pos, size) {}

    Button(Game *game, bool alt, rstr text, vec2 size) : Button(game, alt, text, vec2(), size) {}

    Button(Game *game, bool alt, cstr text, vec2 size) : Button(game, alt, text, vec2(), size) {}

    void reset() override {
        hovered = false;
        pressed = false;
        last_pressed = false;
        just_pressed = false;
    }

    void draw(Batch_Renderer *r) override {
        auto color = vec4(0.9f, 0.9f, 0.9f, 1.0f);

        auto t = tex;
        // TODO
        // if(hovered) t = tex_hovered;
        if(hovered) color = vec4(1.0f);
        if(pressed) t = tex_pressed;
        if(!enabled) color = vec4(0.5f, 0.5f, 0.5f, 1.0f);

        r->push_textured_quad(pos, size, color, t);


        auto text_width = game->font->width(text);
        auto text_color = this->text_color;
        if(hovered) text_color = text_color_hovered;
        if(pressed) text_color = text_color_pressed;
        draw_text(r, game->font, text, pos.x + size.x / 2 - text_width / 2, pos.y + size.y / 2 - game->font->height / 2, text_color);
    }

    void update() override {
        auto mp = game->mouse_pos;
        hovered = enabled && mp.x >= pos.x && mp.y >= pos.y && mp.x < pos.x + size.x && mp.y < pos.y + size.y;
        pressed = hovered && game->mouse[LMB];

        just_pressed = pressed && !last_pressed;
        last_pressed = pressed;
    }
};