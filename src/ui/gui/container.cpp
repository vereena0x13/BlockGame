struct GUI_Container : public Panel {
    Item_Stack held_stack;
    Container *held_stack_container = NULL;
    u32 held_stack_index = 0;

    GUI_Container(Game *game) : Panel(game) {}

    void deinit() override {
        Panel::deinit();
    }

    void draw(Batch_Renderer *r) override {
        Panel::draw(r);

        if(held_stack.is_valid()) {
            vec2 size = vec2(50);
            vec2 pos = game->mouse_pos - size * 0.5f;
            held_stack.draw(r, pos, size);
        }
    }

    void update() override {
        Panel::update();
        handle_slots();
    }

    void on_close() override {
        if(held_stack.is_valid()) {
            held_stack.count -= held_stack_container->insert(held_stack, held_stack_index);
            if(held_stack.is_valid()) {
                // TODO: Probably just drop the item on the ground at this point.
                // Maybe in the future we have a smarter way to handle this, so that
                // items drop to the ground less often, but meh. Ignoring for now lol. 
            }
        }
        held_stack = Item_Stack();
        held_stack_container = NULL;
        held_stack_index = 0;
    }

private:
    void handle_slots() {
        for(auto child : children) {
            auto slot = dynamic_cast<Slot*>(child);
            if(slot && slot->just_pressed) {
                bool alt = game->mouse[RMB];
                auto container = slot->container;
                auto index = slot->index;
                auto stack = container->slots[index];
                if(stack.is_valid()) {
                    if(held_stack.is_valid() && container->accepts(held_stack.item)) {
                        if(stack.item == held_stack.item) {
                            auto s2 = Item_Stack(held_stack.item, alt ? 1 : held_stack.count);
                            held_stack.count -= container->insert(s2, index);
                            if(!held_stack.is_valid()) {
                                held_stack_container = NULL;
                            }
                        } else {
                            auto other = container->remove(index);
                            assert(other.is_valid());

                            if(container->insert(held_stack, index) == held_stack.count) {
                                held_stack = other;
                                held_stack_container = container;
                                held_stack_index = index;
                            } else {
                                // TODO ?
                            }
                        }
                    } else {
                        held_stack = container->remove(index, alt && stack.count > 1 ? stack.count / 2 : stack.count);
                        held_stack_container = container;
                        held_stack_index = index;
                    }
                } else if(held_stack.is_valid()) {
                    auto s2 = Item_Stack(held_stack.item, alt ? 1 : held_stack.count);
                    held_stack.count -= container->insert(s2, index);
                    if(held_stack.is_valid()) {
                        // TODO ?
                    } else {
                        held_stack_container = NULL;
                    }
                }
                break;
            }
        }
    }
};