struct Player : public Entity {
    Camera camera;
    AABB bb;

    Container *inventory;
    Container *hotbar;
    s32 selected_hotbar_slot = 0;

    bool accept_input = true;

    vec3i block_pos;
    vec3i chunk_pos;
    vec3i last_block_pos;
    vec3i last_chunk_pos;
    bool block_pos_changed = false;
    bool chunk_pos_changed = false;

    // NOTE: Only used for debugging
    struct {
        Array<AABB> tested;
        Array<AABB> hit;
    } collision; 

    Texture *tex_hotbar;
    Texture *tex_hotbar_selection;

    Player(Game *game) : Entity(game), camera(Camera(game, this)) {
        inventory = xnew(Container, 27);
        hotbar = xnew(Container, 9);

        tex_hotbar = game->assets->get_texture("ui/hotbar");
        tex_hotbar_selection = game->assets->get_texture("ui/hotbar_selection");
    }

    void scroll_hotbar(f64 y) {
        s32 s = cast(s32, -sign(y));
        selected_hotbar_slot += s;
        if(selected_hotbar_slot < 0) selected_hotbar_slot = hotbar->size - 1;
        if(selected_hotbar_slot > hotbar->size - 1) selected_hotbar_slot = 0;
    }

    Item_Stack get_selected_hotbar_stack() {
        return hotbar->slots[selected_hotbar_slot];
    }

    void deinit() override {
        inventory->deinit();
        xfree(inventory);

        hotbar->deinit();
        xfree(hotbar);

        collision.tested.free();
        collision.hit.free();

        Entity::deinit();
    }

    void update() override {
        TIMED_FUNCTION();
        
        Entity::update();

        camera.update();

        update_aabb();

        if(camera.cursor_grabbed && accept_input) {
            f32 w = glfwGetKey(game->window, GLFW_KEY_W) ? 1.0f : 0.0f;
            f32 s = glfwGetKey(game->window, GLFW_KEY_S) ? 1.0f : 0.0f;
            f32 a = glfwGetKey(game->window, GLFW_KEY_A) ? 1.0f : 0.0f;
            f32 d = glfwGetKey(game->window, GLFW_KEY_D) ? 1.0f : 0.0f;
            f32 down = glfwGetKey(game->window, GLFW_KEY_LEFT_SHIFT) ? 1.0f : 0.0f;
            f32 up = glfwGetKey(game->window, GLFW_KEY_SPACE) ? 1.0f : 0.0f;

            vec3 move = camera.rot.matrix() * vec3(
                d - a,
                0,
                s - w
            );
            move.y = up - down;
            if(move.length() > 0.0f) {
                move = normalize(move);
                this->move(move * (glfwGetKey(game->window, GLFW_KEY_LEFT_CONTROL) ? MOVE_SPEED_SLOW : MOVE_SPEED));
            }
        }


        last_block_pos = block_pos;
        last_chunk_pos = chunk_pos;
        block_pos = vec3i(floor(pos.x), floor(pos.y), floor(pos.z));
        chunk_pos = block_pos >> CHUNK_SIZE_LOG2;
        block_pos_changed = block_pos != last_block_pos;
        chunk_pos_changed = chunk_pos != last_chunk_pos;
    }

    void draw() override {
        Entity::draw();
    }

    void draw(Batch_Renderer *r) {
        TIMED_FUNCTION();

        f32 slot_offset = 16;
        vec2 hotbar_size = vec2(180, 20) * 4;
        vec2 hotbar_pos = vec2(game->window_width / 2, game->window_height) - 
                          vec2(hotbar_size.x / 2, hotbar_size.y);
        r->push_textured_quad(hotbar_pos, hotbar_size, tex_hotbar);
    
        for(u32 i = 0; i < hotbar->size; i++) {
            if(i == selected_hotbar_slot) {
                vec2 selection_size = vec2(22, 22) * 4;
                vec2 selection_pos = hotbar_pos + vec2(i * 80, 0);
                r->push_textured_quad(selection_pos, selection_size, tex_hotbar_selection);
            }

            auto const& slot = hotbar->slots[i];
            if(slot.is_valid()) {
                vec2 slot_size = vec2(50);
                vec2 slot_pos = hotbar_pos + vec2(i * 80 + slot_offset, slot_offset);
                slot.draw(r, slot_pos, slot_size);
            }
        }
    }

    void serialize(ByteBuf *bb) override {
        Entity::serialize(bb);
        camera.serialize(bb);
        inventory->serialize(bb);
        hotbar->serialize(bb);
        bb->write_s32(selected_hotbar_slot);
    }

    void deserialize(ByteBuf *bb) override {
        Entity::deserialize(bb);
        camera.deserialize(bb);
        inventory->deserialize(bb);
        hotbar->deserialize(bb);
        selected_hotbar_slot = bb->read_s32();
    }

private:
    void update_aabb() {
        f32 const s = 0.3f;
        bb = {
            vec3(pos.x - s, pos.y - 1.6f, pos.z - s),
            vec3(pos.x + s, pos.y + 0.2f, pos.z + s)
        };
    }

    void move(vec3 d) {
        TIMED_FUNCTION();
        
        f32 const EPSILON = 0.0001f;
        u32 const MAX_ITER = 8;

        Array<AABB> bbs(temp_allocator);

        collision.tested.clear();
        collision.hit.clear();

        u32 iteration = 0;
        while(d.length() > EPSILON && iteration++ < MAX_ITER) {
            AABB target_bb = bb + d;
            auto broad = AABB::disjunction(bb, target_bb);

            AABB::Hit best = { false, 1.0f };

            vec3i minbp = floor(broad.min);
            vec3i maxbp = floor(broad.max);

            for(s32 i = minbp.x; i <= maxbp.x; i++) {
                for(s32 j = minbp.y; j <= maxbp.y; j++) {
                    for(s32 k = minbp.z; k <= maxbp.z; k++) {
                        auto bp = vec3i(i,j,k);

                        auto block = world->get_block(bp);
                        if(!block || block == block_air) continue;

                        bbs.clear();
                        block->get_aabbs(world, bp, bbs);

                        for(auto const& bbx : bbs) {
                            if(bbx.intersects(broad)) {
                                collision.tested.push(bbx);

                                auto hit = AABB::sweep(bb, bbx, d);
                                if(hit.hit && hit.h < best.h) {
                                    best = hit;
                                    collision.hit.push(bbx);
                                }
                            }
                        }
                    }
                }
            }

            pos += d * (best.h - EPSILON);
            update_aabb();

            if(best.hit) {
                d *= eq(best.n, vec3());
            } else {
                break;
            }
        }
    }
};