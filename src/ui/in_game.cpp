struct Screenshot_Save_Task : public Task {
    cstr path;
    u8 *buf;
    u32 width;
    u32 height;

    Screenshot_Save_Task(cstr _path, u8 *_buf, u32 _width, u32 _height) : path(_path), buf(_buf), width(_width), height(_height) {}

    void run() override {
        if(!stbi_write_png(path, width, height, 3, buf, width * 3)) {
            log(ERROR, "Failed to write screenshot '%s'\n", path);
        } else {
            log(INFO, "Screenshot saved to '%s'", path);
        }
    }

    void deinit() override {
        xfree(path);
        xfree(buf);
        Task::deinit();
    }
};

struct In_Game : public Panel {
    bool show_f3_debug = false;
    bool show_chunk_highlights = false;
    bool show_collision_aabbs = false;
    bool show_hit_uv = false;

    Hit hit;
    u32 chunks_drawn = 0;

    struct {
        GUI_Player_Inventory *player_inv = NULL;
    } gui;
    Panel *active_gui = NULL;

    AABB_Renderer aabb_renderer;

    Sky sky;
    Compass3D compass;

    #ifdef SHADER_HOT_RELOAD
    bool should_reload_shaders = false;
    #endif

    In_Game(Game *game) : Panel(game), sky(Sky(vec3(0.0, 0.0, 1.0))) {
    }

    void deinit() override {
        if(gui.player_inv) {
            gui.player_inv->deinit();
            xfree(gui.player_inv);
        }

        aabb_renderer.deinit();

        sky.deinit();
        compass.deinit();

        Panel::deinit();
    }

    void init_guis() {
        if(gui.player_inv) {
            gui.player_inv->deinit();
            xfree(gui.player_inv);
        }

        gui.player_inv = xnew(GUI_Player_Inventory, game);
    }

    void take_screenshot() {
        TIMED_FUNCTION();
        
        auto w = game->window_width;
        auto h = game->window_height;

        GLuint pbo;
        glCreateBuffers(1, &pbo);
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 3, 0, GL_STREAM_READ);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, 0);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        auto file = tsprintf("screenshots/screenshot_%d-%d-%d_%d-%d-%d.png", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
        auto path = cast(cstr, xalloc(strlen(file) + 1));
        strcpy(path, file);

        u8 *buf = cast(u8*, glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
        auto buf2 = cast(u8*, xalloc(w * h * 3));
        memcpy(buf2, buf, w * h * 3);
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

        game->task_queue.enqueue(xnew(Screenshot_Save_Task, path, buf2, w, h));

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glDeleteBuffers(1, &pbo);
    }

    #ifdef SHADER_HOT_RELOAD
    void reload_shaders() {
        sky.reload_shader();
        game->world->reload_chunk_shader();
        log(INFO, "Shaders reloaded");
    }
    #endif

    void draw_block_highlight() {
        assert(hit.hit);

        auto world = game->world;
        auto block = world->get_block(hit.block_pos);

        Array<AABB> bbs(temp_allocator);
        block->get_aabbs(world, hit.block_pos, bbs);

        assert(hit.aabb_index < bbs.count);
        auto const& bb = bbs[hit.aabb_index];

        aabb_renderer.set_projection(game->perspective); // TODO
        aabb_renderer.set_view(game->player->camera.view());
        aabb_renderer.draw({ bb.min - vec3(0.00125f), bb.max + vec3(0.00125f) }, vec4(0.8f));
    }

    void draw_sky() {
        sky.set_projection(game->perspective); // TODO
        sky.set_view(game->player->camera.rot.matrix());
        sky.set_view_pos(game->player->pos);
        sky.draw();
    }

    void draw_compass3d() {
        f32 scale = 0.01f;
        f32 dist = 0.1f;
        vec3 dir = game->player->camera.dir();
        mat4 model = mat4::scale(scale, scale, scale) *
                     mat4::translate(game->player->pos.x, game->player->pos.y, game->player->pos.z) *
                     mat4::translate(dir.x * dist, dir.y * dist, dir.z * dist);
        compass.set_projection(game->perspective);
        compass.set_view(game->player->camera.view());
        compass.set_model(model);
        compass.draw();
    }

    void draw_chunk_highlights() {
        auto player = game->player;
        auto block_pos = floor(player->pos);
        auto chunk_pos = block_pos >> CHUNK_SIZE_LOG2;

        aabb_renderer.set_projection(game->perspective); // TODO
        aabb_renderer.set_view(game->player->camera.view());

        s32 const N = 2;
        for(s32 i = -N; i <= N; i++) {
            for(s32 j = -N; j <= N; j++) {
                for(s32 k = -N; k <= N; k++) {
                    auto min = (chunk_pos + vec3i(i,j,k)) * CHUNK_SIZE;
                    auto max = min + CHUNK_SIZE;
                    aabb_renderer.draw({ flt(min), flt(max) });
                }
            }
        }
    }

    void draw_collision_aabbs() {
        auto player = game->player;

        aabb_renderer.set_projection(game->perspective); // TODO
        aabb_renderer.set_view(game->player->camera.view());

        for(auto const& bb : player->collision.tested) {
            aabb_renderer.draw({ bb.min - vec3(0.00125f), bb.max + vec3(0.00125f) }, vec4(1, 1, 0, 1));
        }

        for(auto const& bb : player->collision.hit) {
            aabb_renderer.draw({ bb.min - vec3(0.00125f), bb.max + vec3(0.00125f) }, vec4(1, 0, 0, 1));
        }
    }

    void draw_hit_uv(Batch_Renderer *r) {
        auto const& bp = hit.block_pos;
        auto const& pos = hit.pos;
        auto const& face = hit.face;
        s32 axis = hit.axis;
        s32 axisu = hit.axisu;
        s32 axisv = hit.axisv;
        bool flipu = hit.flipu;
        bool flipv = hit.flipv;
        f32 fu = hit.fu;
        f32 fv = hit.fv;
        auto const& uv = hit.uv;
        vec3 n = flt(DIRECTION_OFFSET[face]);

        vec3 quad[4];
        for(u32 v = 0; v < 4; v++) {
            f32 s = 0.025f;
            vec3 m = AXIS_VEC3[AXIS_OTHER_AXES[axis][0]] + AXIS_VEC3[AXIS_OTHER_AXES[axis][1]];
            quad[v] = CUBE_VERTICES[CUBE_INDICES[face][UNIQUE_INDICES[v]]] * (m * s) - (m * s * 0.5f) +
                        bp + vec3(0.5f) +
                        n * 0.5f -
                        AXIS_VEC3[axisu] * 0.5f * fu -
                        AXIS_VEC3[axisv] * 0.5f * fv +
                        AXIS_VEC3[axisu] * uv.x * fu +
                        AXIS_VEC3[axisv] * uv.y * fv +
                        n * 0.005f;
        }
        r->push_solid_quad(quad[0], quad[1], quad[2], quad[3], vec4(1, 0, 0, 1));
    }

    void draw_3d_culled() {
        draw_sky();

        chunks_drawn = game->world->draw_chunks(game->wireframe);

        if(hit.hit && !active_gui) {
            draw_block_highlight();
        }

        if(show_f3_debug) {
            draw_compass3d();
        }

        if(show_chunk_highlights) {
            draw_chunk_highlights();
        }

        if(show_collision_aabbs) {
            draw_collision_aabbs();
        }
    }

    void draw_3d(Batch_Renderer *r) {
        if(hit.hit && show_hit_uv) {
            draw_hit_uv(r);
        }
    }

    void draw(Batch_Renderer *r) override {
        Panel::draw(r);

        auto const& font = game->font;
        auto const& player = game->player;

        player->draw(r);

        if(show_f3_debug) {
            draw_text(r, font, tsprintf("FPS: %u", game->fps), 10, 10, vec4(1));
            draw_text(r, font, tsprintf("XYZ: %0.2f %0.2f %0.2f", player->pos.x, player->pos.y, player->pos.z), 10, 30, vec4(1));
            draw_text(r, font, tsprintf("cXYZ: %d %d %d", player->chunk_pos.x, player->chunk_pos.y, player->chunk_pos.z), 10, 50, vec4(1));
            draw_text(r, font, tsprintf("Pitch: %0.2f", player->camera.pitch), 10, 70, vec4(1));
            draw_text(r, font, tsprintf("Yaw: %0.2f", player->camera.yaw), 10, 90, vec4(1));
            draw_text(r, font, tsprintf("Sun: %0.2f", sky.sun.angle), 10, 110, vec4(1));
            auto pdir = angle_to_direction(player->camera.yaw);
            draw_text(r, font, tsprintf("Dir: %s (%d,%d,%d)", DIRECTION_NAME[pdir], DIRECTION_OFFSET[pdir].x, DIRECTION_OFFSET[pdir].y, DIRECTION_OFFSET[pdir].z), 10, 130, vec4(1));
        } else {
            auto w2 = game->window_width / 2;
            auto h2 = game->window_height / 2;
            r->push_solid_quad(w2 - 5, h2 - 5, 10, 10, vec4(1,1,1,0.5f));
        }

        if(active_gui) {
            r->push_solid_quad(0, 0, game->window_width, game->window_height, vec4(0, 0, 0, 0.5f));
            active_gui->draw(r);
        }
    }

    void update() override {
        #ifdef SHADER_HOT_RELOAD
        if(should_reload_shaders) {
            should_reload_shaders = false;
            reload_shaders();
        }
        #endif

        auto world = game->world;
        auto player = game->player;

        player->accept_input = active_gui == NULL;

        world->update();

        if(hit.hit && !game->show_imgui && !active_gui) {
            if(game->mouse_pressed(LMB)) {
                world->set_block(hit.block_pos, block_air->id);
            } else if(game->mouse_pressed(RMB)) {
                vec3i bp = hit.block_pos + DIRECTION_OFFSET[hit.face];
                auto stack = player->get_selected_hotbar_stack();
                if(stack.is_valid()) {
                    auto item_block = dynamic_cast<Item_Block*>(stack.item);
                    if(item_block) {
                        auto id = item_block->block->id;
                        world->set_block(bp, id);
                        BLOCKS[id]->on_placed(world, bp, &hit, player->camera.yaw); 
                    }
                }
            }
        }

        vec3 camera_dir = player->camera.dir();
        hit = raycast(world, player->pos, camera_dir, 7.0f);

        if(active_gui) active_gui->update();

        Panel::update();
    }

    void key_callback(s32 key, s32 scancode, s32 action, s32 mods) override {
        if(action == GLFW_RELEASE) {
            switch(key) {
                case GLFW_KEY_E: {
                    if(active_gui) set_gui(NULL);
                    else           set_gui(gui.player_inv);
                    return;
                }
                case GLFW_KEY_F2: {
                    take_screenshot();
                    return;
                }
                case GLFW_KEY_F3: {
                    show_f3_debug = !show_f3_debug;
                    return;
                }
                case GLFW_KEY_F8: {
                    show_chunk_highlights = !show_chunk_highlights;
                    return;
                }
                #ifdef SHADER_HOT_RELOAD
                case GLFW_KEY_F9: {
                    should_reload_shaders = true;
                    return;
                }
                #endif
                case GLFW_KEY_ESCAPE: {
                    if(active_gui) {
                        set_gui(NULL);
                    } else {
                        game->set_gui(GUI_PAUSE);
                        game->world->save_all();
                    }
                    return;
                }
                default: {
                    if(!active_gui && key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
                        game->player->selected_hotbar_slot = key - GLFW_KEY_1;
                        return;
                    }
                }
            }
        }
        
        Panel::key_callback(key, scancode, action, mods);
    }

    void scroll_callback(f64 x, f64 y) override {
        if(!active_gui) {
            game->player->scroll_hotbar(y);
        }
    }

    void mouse_pos_callback(vec2 mouse_pos, vec2 mouse_delta) override {
        auto player = game->player;
        if(player->accept_input) player->camera.mouse_moved(mouse_delta.x, mouse_delta.y);
    }

    void set_gui(Panel *gui) {
        if(active_gui) active_gui->on_close();
        active_gui = gui;
        
        if(active_gui) {
            active_gui->reset();
            game->player->camera.release_cursor();
        } else {
            game->player->camera.grab_cursor();
        }
    }
};