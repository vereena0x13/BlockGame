struct Text_Box : public Component {
    vec2 pos;
    vec2 size;

    Texture *tex;

    Array<u8> chars;
    bool focused = false;
    u32 cursor_timer = 0;

    Text_Box(Game *game, vec2 _size) : Component(game), size(_size) {
        tex = game->assets->get_texture("ui/container");
    }

    void deinit() override {
        chars.free();
    }

    void draw(Batch_Renderer *r) override {
        Component::draw(r);

        r->push_textured_quad(pos, size, tex);

        str s = value();
        if(focused && cursor_timer >= 50) s = tsprintf("%s|", s);
        auto sw = game->font->width(s);
        auto sh = game->font->height;
        auto p = pos + size * 0.5f - vec2(sw * 0.5f, sh * 0.5f);
        draw_text(r, game->font, s, p.x, p.y, vec4(1, 1, 1, 1));
    }

    void update() override {
        cursor_timer++;
        if(cursor_timer == 100) cursor_timer = 0;

        if(game->mouse_pressed(LMB)) {
            auto const& mp = game->mouse_pos;
            focused = mp.x >= pos.x && mp.y >= pos.y && mp.x < pos.x + size.x && mp.y < pos.y + size.y;
        }
    }

    void key_callback(s32 key, s32 scancode, s32 action, s32 mods) override {
        if(!focused) return;

        if(key == GLFW_KEY_BACKSPACE && action == GLFW_RELEASE && chars.count > 0) {
            chars.pop();
        }
    }

    void char_callback(u32 cp) override {
        if(cp >= 32 && cp <= 126) {
            chars.push(cp);
        }
    }

    str value() {
        str s = mkstr(NULL, chars.count, temp_allocator);
        for(u32 i = 0; i < chars.count; i++) s[i] = chars[i];
        return s;
    }
};