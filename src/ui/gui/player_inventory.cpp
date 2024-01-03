struct GUI_Player_Inventory : public GUI_Container {
    vec2 pos;
    vec2 size;

    Slot *inv_slots[9*3];
    Slot *hotbar_slots[9];

    GUI_Player_Inventory(Game *game) : GUI_Container(game), size(vec2(700, 365)) {
        vec2 s = vec2(60, 60);

        for(u32 i = 0; i < 9; i++) {
            for(u32 j = 0; j < 3; j++) {
                u32 k = i + j * 9;
                inv_slots[k] = xnew(Slot, game, s, game->player->inventory, k);
                children.push(inv_slots[k]);
            }
        }

        for(u32 i = 0; i < 9; i++) {
            hotbar_slots[i] = xnew(Slot, game, s, game->player->hotbar, i);
            children.push(hotbar_slots[i]);
        }
    }

    void draw(Batch_Renderer *r) override {
        auto ws = vec2(game->window_width, game->window_height);
        auto hws = ws * 0.5f;

        pos = hws - size * 0.5f;


        vec2 slot_border = vec2(15);
        vec2 slots_offset = vec2(20, 20);

        for(u32 i = 0; i < 9; i++) {
            for(u32 j = 0; j < 3; j++) {
                u32 k = i + j * 9;
                auto slot = inv_slots[k];
                slot->pos = pos + vec2(i, j) * (slot->size + slot_border) + slots_offset;
            }
        }

        for(u32 i = 0; i < 9; i++) {
            auto slot = hotbar_slots[i];
            slot->pos = pos + vec2(i, 0) * (slot->size + slot_border) + slots_offset + vec2(0, slot->size.y + slot_border.y) * 3.5f;
        }


        r->push_solid_quad(pos, size, vec4(0.7f, 0.7f, 0.7f, 1.0f));

        GUI_Container::draw(r);
    }
};