#define GL_MAJOR 4
#define GL_MINOR 6
#define GL_DEBUG
//#define IMGUI_DISABLE


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif


#define GLEW_NO_GLU
#include "GL/glew.h"


#include "GLFW/glfw3.h"


#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


#define RND_IMPLEMENTATION
#include "rnd.h"


#include "noise1234.c"


#include "meow_hash_x64_aesni.h"


#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui_impl_opengl3.h"
#include "implot.h"
#endif

#ifndef IMGUI_DISABLE
#include "imgui_demo.cpp"
#include "implot_demo.cpp"
#include "imgui_support.cpp"
#endif


#define TEMPORARY_STORAGE_SIZE MEBIBYTES(1)
#define VSTD_IMPL
#include "vstd.hpp"


// https://gist.github.com/statico/6809850727c708f08458
#ifdef _WIN32
extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;
#endif


pthread_t main_thread;


#include "util/off_the_rails.cpp"
#include "util/atomics.cpp"
#include "util/ticket_mutex.cpp"
#include "util/pool.hpp"
#include "util/pool.cpp"
#include "util/math.cpp"
#include "util/profiler.cpp"
#include "util/log.hpp"
#include "util/glutil.cpp"
#include "util/fsutil.cpp"
#include "util/log.cpp"
#include "util/cube_data.hpp"
#include "util/utf8.cpp"
#include "util/throttle.cpp"
#include "util/task_queue.cpp"
#include "util/static_bitset.cpp"
#include "render/texture.cpp"
#include "render/texture_array.cpp"
#include "render/shader.cpp"
#include "render/buffer_objects.cpp"
#include "render/batch_renderer.cpp"
#include "render/font.cpp"
#include "render/compass3d.cpp"
#include "assets.cpp"
#include "game.hpp"
#include "block/block.hpp"
#include "block/block_entity.hpp"
#include "world/chunk.hpp"
#include "world/chunk_mesh.hpp"
#include "world/world_storage.hpp"
#include "world/world_generator.hpp"
#include "world/sky.hpp"
#include "world/world.hpp"
#include "render/aabb_renderer.cpp"
#include "item/item.hpp"
#include "item/container.hpp"
#include "item/item.cpp"
#include "entity/entity.cpp"
#include "render/camera.cpp"
#include "entity/player.cpp"
#include "block/block.cpp"
#include "world/chunk.cpp"
#include "world/chunk_mesh.cpp"
#include "world/world_storage.cpp"
#include "world/world_generator.cpp"
#include "world/world.cpp"
#include "util/raycast.cpp"
#include "ui/component.cpp"
#include "ui/panel.cpp"
#include "ui/button.cpp"
#include "ui/text_box.cpp"
#include "ui/slot.cpp"
#include "ui/gui/container.cpp"
#include "ui/gui/player_inventory.cpp"
#include "ui/main_menu.cpp"
#include "ui/new_world.cpp"
#include "ui/load_world.cpp"
#include "ui/in_game.cpp"
#include "ui/pause.cpp"
#include "ui/settings.cpp"


#ifdef GL_DEBUG
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == GL_DEBUG_SEVERITY_LOW) {
        return;
    }

    auto level = WARN;
    if(severity = GL_DEBUG_SEVERITY_HIGH) level = ERROR;
    log(level, "%s", message);
}
#endif


void scroll_callback(GLFWwindow *window, f64 x, f64 y) {
    auto p = cast(Game*, glfwGetWindowUserPointer(window));
    p->scroll_callback(x, y);
}

void mouse_pos_callback(GLFWwindow *window, f64 x, f64 y) {
    auto p = cast(Game*, glfwGetWindowUserPointer(window));
    p->mouse_pos_callback(x, y);
}

void mouse_button_callback(GLFWwindow *window, s32 button, s32 action, s32 mods) {
    auto p = cast(Game*, glfwGetWindowUserPointer(window));
    p->mouse_button_callback(button, action, mods);
}

void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
    auto p = cast(Game*, glfwGetWindowUserPointer(window));
    p->key_callback(key, scancode, action, mods);
}

void char_callback(GLFWwindow *window, u32 cp) {
    auto p = cast(Game*, glfwGetWindowUserPointer(window));
    p->char_callback(cp);
}

void window_size_callback(GLFWwindow* window, s32 width, s32 height) {
    auto p = cast(Game*, glfwGetWindowUserPointer(window));
    p->window_size_callback(width, height);
}

void Game::scroll_callback(f64 x, f64 y) {
    #ifndef IMGUI_DISABLE
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse) return;
    #endif

    guis[active_gui]->scroll_callback(x, y);
}

void Game::mouse_pos_callback(f64 x, f64 y) {
    if(first_mouse) {
        first_mouse = false;
        mouse_delta = vec2(x, y);
    } else {
        mouse_delta = vec2(mouse_pos.x - x, mouse_pos.y - y);
    }

    mouse_pos = vec2(x, y);

    guis[active_gui]->mouse_pos_callback(mouse_pos, mouse_delta);
}

void Game::mouse_button_callback(s32 button, s32 action, s32 mods) {
    #ifndef IMGUI_DISABLE
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse) return;
    #endif

    bool pressed = action == GLFW_PRESS;
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mouse[LMB] = pressed;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            mouse[MMB] = pressed;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mouse[RMB] = pressed;
            break;
    }
}

void Game::key_callback(s32 key, s32 scancode, s32 action, s32 mods) {
    if(action == GLFW_RELEASE) {
        switch(key) {
            case GLFW_KEY_F11: {
                fullscreen = !fullscreen;
                fullscreen_changed = true;
                break;
            }
            #ifdef PROFILER_ENABLED
            case GLFW_KEY_F6: {
                show_profiler = !show_profiler;
                break;
            }
            case GLFW_KEY_F7: {
                paused_for_profiler = !paused_for_profiler;
                break;
            }
            #endif
            case GLFW_KEY_F12: {
                show_imgui = !show_imgui;
                break;
            }
            default: {
                if(active_gui == GUI_IN_GAME && show_imgui && key == GLFW_KEY_ESCAPE) {
                    if(player->camera.cursor_grabbed) player->camera.release_cursor();
                    else player->camera.grab_cursor();
                    return;
                }

                #ifndef IMGUI_DISABLE
                ImGuiIO& io = ImGui::GetIO();
                if(io.WantCaptureKeyboard) return;
                #endif

                guis[active_gui]->key_callback(key, scancode, action, mods);
                break;
            }
        }
    }
}

void Game::char_callback(u32 cp) {
    guis[active_gui]->char_callback(cp);
}

void Game::window_size_callback(s32 width, s32 height) {
    window_width = width;
    window_height = height;
    window_resized = true;
}

bool Game::mouse_pressed(s32 button) {
    return mouse[button] && !mouse_last[button];
}

bool Game::init_gl() {
    ////////////////////////////////////////////////
    // Set up GLFW and a window, GLEW, and OpenGL //
    ////////////////////////////////////////////////

    if (!glfwInit()) {
        log(ERROR, "Failed to initialized GLFW3!\n");
        return false;
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // TODO: query GL_MAX_SAMPLES

    #ifdef GL_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif

    window = glfwCreateWindow(window_width, window_height, "BlockGame", 0, 0);
    if(!window) {
        log(ERROR, "Failed to create GLFW window!\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, ::scroll_callback);
    glfwSetCursorPosCallback(window, ::mouse_pos_callback);
    glfwSetMouseButtonCallback(window, ::mouse_button_callback);
    glfwSetKeyCallback(window, ::key_callback);
    glfwSetCharCallback(window, ::char_callback);
    glfwSetWindowSizeCallback(window, ::window_size_callback);

    glfwSwapInterval(1);

    log(DEBUG, "GLFW initialized");

    if(glewInit() != GLEW_OK) {
        log(ERROR, "Failed to initialize GLEW!\n");
        return false;
    }

    log(DEBUG, "GLEW initialized");

    #ifdef GL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB) gl_debug_callback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        log(DEBUG, "OpenGL debug output enabled");
    #endif

    dump_gl_info();
    //dump_gl_extensions();

    glClearColor(0, 0, 0, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_MULTISAMPLE);

    log(DEBUG, "OpenGL initialized");

    return true;
}

bool Game::init() {
    if(!init_gl()) return false;

    #ifndef IMGUI_DISABLE
    imgui_init(window);
    #endif

    assets = xnew(Assets);

    batch_renderer = xnew(Batch_Renderer);
    batch_renderer->init();

    font = load_font("assets/fonts/Meslo LG L Regular Nerd Font Complete.ttf", 24.0f);

    
    register_blocks();
    register_items();
    register_textures();

    
    guis[GUI_MAIN_MENU] = gui.main_menu = xnew(Main_Menu, this);
    guis[GUI_NEW_WORLD] = gui.new_world = xnew(New_World, this);
    guis[GUI_LOAD_WORLD] = gui.load_world = xnew(Load_World, this);
    guis[GUI_IN_GAME] = gui.in_game = xnew(In_Game, this);
    guis[GUI_PAUSE] = gui.pause = xnew(Pause, this);
    guis[GUI_SETTINGS] = gui.settings = xnew(Settings, this);
    set_gui(GUI_MAIN_MENU);

    mkdir_if_not_exists("saves");
    mkdir_if_not_exists("screenshots");

    stbi_flip_vertically_on_write(true);

    return true;
}

void Game::register_textures() {
    block_textures = xnew(Texture_Array, 16, 16, 256, 5); // TODO: magic numbers
    item_textures = xnew(Texture_Array, 16, 16, 256);
    for(u32 i = 0; BLOCKS[i]; i++) BLOCKS[i]->register_textures(block_textures);
    for(u32 i = 0; ITEMS[i]; i++) ITEMS[i]->register_textures(item_textures);
    log(DEBUG, "Block and Item textures registered");
}

void Game::generate_item_block_textures() {
    auto const& chunk_shader = world->shader;

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    Vertex_Array block_item_vao;
    Vertex_Buffer block_item_vbo;
    Index_Buffer block_item_ibo;
    Frame_Buffer block_item_fbo;

    block_item_fbo.init();
    block_item_vbo.init(sizeof(Chunk_Mesh::Vtx) * 4 * 3, GL_STATIC_DRAW);
    block_item_ibo.init(sizeof(u32) * 6 * 3, GL_STATIC_DRAW);
    block_item_vao.init();
    Chunk_Mesh::Vtx::configure_vao(block_item_vao, block_item_vbo, block_item_ibo);

    block_item_fbo.bind();
    block_item_vao.bind();
    chunk_shader->bind();
    
    f32 f = M_PI / 2; // NOTE: No one knows why this is PI/2.
    chunk_shader->set_uniform_mat4(game->world->u_proj, mat4::ortho(-f, f, f, -f, -10, 10));
    chunk_shader->set_uniform_mat4(
        game->world->u_view, 
        mat4::rotate(45.0f, vec3(0.0f, 1.0f, 0.0f)) *
        mat4::rotate(30.0f, vec3(-1.0f, 0.0f, 0.0f))
    );
    chunk_shader->set_uniform_vec3(game->world->u_view_pos, vec3());

    game->block_textures->bind(0);

    u32 count = 0;
    for(u32 i = 0; ITEMS[i]; i++) {
        ITEMS[i]->generate_block_textures(&block_item_fbo, &block_item_vbo, &block_item_ibo);
        count++;
    }

    chunk_shader->unbind();
    block_item_vao.unbind();
    block_item_fbo.unbind();

    block_item_vao.deinit();
    block_item_vbo.deinit();
    block_item_ibo.deinit();
    block_item_fbo.deinit();

    glViewport(vp[0], vp[1], vp[2], vp[3]);

    log(DEBUG, "%d Item_Block textures generated", count);
}

void Game::check_resize() {
    if(window_resized) {
        window_resized = false;

        update_matrices_and_frustum();

        glViewport(0, 0, window_width, window_height);

        log(TRACE, "Window resized: (%d, %d)", window_width, window_height);
    }
}

void Game::check_fullscreen() {
    if(fullscreen_changed) {
        fullscreen_changed = false;

        static s32 wx, wy, ww, wh;
        if(fullscreen) {
            glfwGetWindowPos(window, &wx, &wy);
            glfwGetWindowSize(window, &ww, &wh);

            auto monitor = glfwGetPrimaryMonitor();
            auto mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, 0);

            log(TRACE, "Fullscreen enabled (%d, %d)", mode->width, mode->height);
        } else {
            glfwSetWindowMonitor(window, nullptr, wx, wy, ww, wh, 0);
            log(TRACE, "Fullscreen disabled");
        }
    }
}

void Game::update_matrices_and_frustum() {
    perspective = mat4::perspective(fov, (f32)window_width / (f32)window_height, near, far);
    orthographic = mat4::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, -1.0f, 1.0f);
    if(world) world->create_view_frustum();
}

s32 Game::run(s32 argc, cstr *argv) {
    if(!init()) return 1;

    s32 res = 0;
    f64 last_time = glfwGetTime();
    f64 timer = last_time;
    u32 frames = 0;

    #ifndef IMGUI_DISABLE
    ImGuiIO& io = ImGui::GetIO();
    #endif

    while (!glfwWindowShouldClose(window) && !quit) {
        #ifdef PROFILER_ENABLED
        if(!paused_for_profiler) profiler.begin_frame();
        #endif

        // Timing
        f64 now = glfwGetTime();
        dt = now - last_time;
        last_time = now;

        if(glfwGetTime() - timer >= 1) {
            fps = frames;
            frames = 0;
            timer += 1;
        }

        glfwPollEvents();
        check_resize();
        check_fullscreen();

        throttles.reset();

        // TODO: set a fixed tick rate
        update();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        #ifndef IMGUI_DISABLE
        imgui_begin_frame();
        #endif

        render();

        #ifndef IMGUI_DISABLE
        //if(active_gui == GUI_IN_GAME) gui.in_game->draw_imgui(br_stats);
        if(show_imgui) {
            draw_imgui();

            if(show_imgui_demo_window) ImGui::ShowDemoWindow();
            if(show_implot_demo_window) ImPlot::ShowDemoWindow();
            
            #ifdef PROFILER_ENABLED
            if(show_profiler) profiler.show();
            #endif
        }
        #endif

        // Swap Buffers and Frame Cleanup
        #ifndef IMGUI_DISABLE
        imgui_end_frame();
        #endif
        glfwSwapBuffers(window);
        frames++;

        for(u32 i = 0; i < array_length(mouse); i++) {
            mouse_last[i] = mouse[i];
        }

        #ifdef PROFILER_ENABLED
        if(paused_for_profiler) profiler.clear_frame_events();
        else profiler.end_frame();
        #endif

        treset();
    }

    shutdown();
    return res;
}

void Game::update() {
    TIMED_BLOCK("Update");
    
    #ifdef PROFILER_ENABLED
    if(!paused_for_profiler) guis[active_gui]->update();
    #else
    guis[active_gui]->update();
    #endif
}

void Game::render() {
    TIMED_BLOCK("Render");

    {
        TIMED_BLOCK("3D Pass");

        if(active_gui == GUI_IN_GAME || active_gui == GUI_PAUSE) {
            gui.in_game->draw_3d_culled();
        }
    }
    
    {
        TIMED_BLOCK("3D Dynamic Pass");

        glDisable(GL_CULL_FACE);
        if(player) {
            batch_renderer->set_projection(perspective);
            batch_renderer->begin(player->camera.view());
            {
                if(active_gui == GUI_IN_GAME || active_gui == GUI_PAUSE) {
                    gui.in_game->draw_3d(batch_renderer);
                }
            }
            batch_renderer->end();
        }
    }

    {
        TIMED_BLOCK("2D Pass");
        
        glDisable(GL_DEPTH_TEST);
        batch_renderer->set_projection(orthographic);
        batch_renderer->begin(mat4());
        {
            if(active_gui == GUI_PAUSE) gui.in_game->draw(batch_renderer);
            guis[active_gui]->draw(batch_renderer);
        }
        br_stats = batch_renderer->end_frame();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
}

#ifndef IMGUI_DISABLE
void Game::draw_imgui() {
    if(!show_imgui) return;

    auto const& world = game->world;
    auto const& player = game->player;
    ImGuiIO& io = ImGui::GetIO();

    if(ImGui::BeginMainMenuBar()) {
        #ifdef PROFILER_ENABLED
        if(ImGui::BeginMenu("Profiler")) {
            ImGui::Checkbox("Show", &show_profiler);
            ImGui::Checkbox("Pause", &paused_for_profiler);

            ImGui::EndMenu();
        }
        #endif

        ImGui::EndMainMenuBar();
    }

    if(ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
        if(ImGui::BeginTabBar("##Tabs")) {
            if(ImGui::BeginTabItem("Misc")) {
                #ifdef BG_DEBUG
                ImGui::Text("Build: DEBUG");
                #else
                ImGui::Text("Build: RELEASE");
                #endif
                ImGui::Text("Window Size: (%d, %d)", game->window_width, game->window_height);
                ImGui::Text("FPS: %.1f", io.Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
                ImGui::EndTabItem();
            }

            if(active_gui == GUI_IN_GAME) {
                if(ImGui::BeginTabItem("Player")) {
                    ImGui::Text("Pos: (%0.3f, %0.3f, %0.3f)", player->pos.x, player->pos.y, player->pos.z);
                    auto chunk_pos = vec3i(player->pos.x, player->pos.y, player->pos.z) >> CHUNK_SIZE_LOG2;
                    ImGui::Text("Chunk: (%d, %d, %d)", chunk_pos.x, chunk_pos.y, chunk_pos.z);
                    ImGui::Text("Pitch: %0.2f", player->camera.pitch);
                    ImGui::Text("Yaw: %0.2f", player->camera.yaw);

                    auto const& hit = gui.in_game->hit;
                    ImGui::Separator();
                    ImGui::Text("Hit: %d", hit.hit);
                    ImGui::Text("Iterations: %u", hit.iters);
                    ImGui::Text("Dir: (%0.8f, %0.8f, %0.8f)", player->camera.dir().x, player->camera.dir().y, player->camera.dir().z);
                    ImGui::Text("Dist: %0.3f", hit.dist);
                    ImGui::Text("Pos: (%0.3f, %0.3f, %0.3f)", hit.pos.x, hit.pos.y, hit.pos.z);
                    ImGui::Text("Block Pos: (%d, %d, %d)", hit.block_pos.x, hit.block_pos.y, hit.block_pos.z);
                    ImGui::Text("Face Normal: (%d, %d, %d)", 
                        DIRECTION_OFFSET[hit.face].x,
                        DIRECTION_OFFSET[hit.face].y,
                        DIRECTION_OFFSET[hit.face].z
                    );
                    ImGui::Text("Face: %s", DIRECTION_NAME[hit.face]);
                    ImGui::Text("AABB: %d", hit.aabb_index);
                    ImGui::Text("UV: (%0.2f, %0.2f)", hit.uv.x, hit.uv.y);
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("World")) {
                    ImGui::Text("Total Chunks: %d", world->chunks.count);
                    ImGui::Text("Chunks Drawn: %d", gui.in_game->chunks_drawn);
                    ImGui::EndTabItem();
                }
            }

            if(ImGui::BeginTabItem("Batch Renderer")) {
                ImGui::Text("Quads: %u", br_stats.quads);
                ImGui::Text("Vertices: %u", br_stats.vertices);
                ImGui::Text("Indices: %u", br_stats.indices);
                ImGui::Text("Draw Calls: %u", br_stats.draw_calls);
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Memory")) {
                ImGui::Text("Temp: %u / %u", temporary_storage.used, TEMPORARY_STORAGE_SIZE);
                ImGui::Text("Temp Max: %u", temporary_storage.high_water_mark);
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Throttles")) {
                ImGui::Text("chunk_mesh_gen: %d / %d", throttles.chunk_mesh_gen.count, throttles.chunk_mesh_gen.max);
                ImGui::Text("region_save: %d / %d", throttles.region_save.count, throttles.region_save.max);
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Settings")) {
                if(ImGui::Checkbox("V-SYNC", &game->vsync)) {
                    if(game->vsync) glfwSwapInterval(1);
                    else            glfwSwapInterval(0);
                }
                ImGui::Checkbox("Show ImGui Demo Window", &show_imgui_demo_window);
                ImGui::Checkbox("Show ImPlot Demo Window", &show_implot_demo_window);
                if(active_gui == GUI_IN_GAME) {
                    if(ImGui::SliderInt("FOV", &game->fov, 45, 135)) game->update_matrices_and_frustum();
                    ImGui::Checkbox("Wireframe", &game->wireframe);
                    ImGui::Checkbox("Show Collision AABBs", &gui.in_game->show_collision_aabbs);
                    ImGui::Checkbox("Show Hit UV", &gui.in_game->show_hit_uv);
                    ImGui::Checkbox("Show ImGui Metrics", &game->show_imgui_metrics_window);
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        if(game->show_imgui_metrics_window) {
            ImGui::ShowMetricsWindow();
        }
    }
    ImGui::End();
}
#endif

void Game::set_gui(GuiID id) {
    guis[active_gui]->on_close();
    active_gui = id;
    guis[active_gui]->reset();

    for(u32 i = 0; i < array_length(mouse); i++) {
        mouse[i] = false;
        mouse_last[i] = false;
    }

    switch(id) {
        case GUI_IN_GAME:
            gui.in_game->init_guis();
            player->camera.grab_cursor();
            break;
        case GUI_PAUSE:
            player->camera.release_cursor();
            break;
    }
}

void Game::push_gui() {
    gui_stack.push(active_gui);
}

void Game::pop_gui() {
    set_gui(gui_stack.pop());
}

void Game::load_world(str name) {
    log(INFO, "Loading world '%s'", name);

    bool is_new = !world_exists(name);
    world = xnew(World, name);

    // TODO: refactor this; we need the chunk shader for 
    // generate_item_block_textures and right now we 
    // create that in World.
    if(!generated_item_block_textures) {
        generated_item_block_textures = true;
        generate_item_block_textures();
    }

    player = xnew(Player, this);
    player->pos = vec3(0, 96, 0);
    world->set_player(player);
    world->load();


    // TODO remove this
    if(is_new) {
        player->hotbar->slots[0] = Item_Stack(item_stone, 50);
        player->hotbar->slots[1] = Item_Stack(item_dirt, 50);
        player->hotbar->slots[2] = Item_Stack(item_grass, 50);
        player->hotbar->slots[3] = Item_Stack(item_sand, 50);
        player->hotbar->slots[4] = Item_Stack(item_glass, 50);
        player->hotbar->slots[5] = Item_Stack(item_furnace, 50);
        player->hotbar->slots[6] = Item_Stack(item_iron_block, 50);
        player->hotbar->slots[7] = Item_Stack(item_gold_block, 50);
    }
}

void Game::unload_world() {
    world->save_all();
    
    world->deinit();
    xfree(world);
    world = NULL;

    player = NULL;
}

bool Game::world_exists(str name) {
    char path[256];
    sprintf(path, "saves/%s", name);

    auto dir = opendir(path);
    if(dir) {
        closedir(dir);
        return true;
    }

    return false;
}

void Game::shutdown() {
    task_queue.deinit();
    
    assets->deinit();
    xfree(assets);

    batch_renderer->deinit();
    xfree(batch_renderer);

    free_font(font);

    block_textures->deinit();
    item_textures->deinit();

    for(u32 i = 0; i < NUM_GUI_IDS; i++) {
        guis[i]->deinit();
        xfree(guis[i]);
    }

    gui_stack.free();

    #ifndef IMGUI_DISABLE
    imgui_shutdown();
    #endif

    #ifdef PROFILER_ENABLED
    profiler.deinit();
    #endif

    glfwTerminate();
}


Game *game;

s32 main(s32 argc, cstr *argv) {
    main_thread = pthread_self();
    log_init();
    log(DEBUG, "Process ID: %d", getpid());
    Game _game;
    game = &_game;
    s32 r = game->run(argc, argv);
    log(INFO, "Game shutdown complete. Goodbye!");
    log_deinit();
    return r;
}