#ifndef GAME_H
#define GAME_H


#define LMB 0
#define MMB 1
#define RMB 2


enum GuiID : u32 {
    GUI_MAIN_MENU,
    GUI_NEW_WORLD,
    GUI_LOAD_WORLD,
    GUI_IN_GAME,
    GUI_PAUSE,
    GUI_SETTINGS,
    NUM_GUI_IDS
};


enum GameMode : u8 {
    SURVIVAL,
    CREATIVE
};


struct Game {
    GLFWwindow *window;

    s32 window_width        = 1280;
    s32 window_height       = 720;
    bool window_resized     = true;

    bool fullscreen         = false;
    bool fullscreen_changed = false;

    bool quit = false;
    bool vsync = true;

    s32 fov = 90;
    f32 near = 0.1f;
    f32 far = 2000.0f;

    vec2 mouse_pos;
    vec2 mouse_delta;
    bool first_mouse = true;
    bool mouse[6] = { false, false, false, false, false, false };
    bool mouse_last[6] = { false, false, false, false, false, false };

    s32 gamepad = GLFW_JOYSTICK_3; // TODO: don't hardcode -1;

    bool generated_item_block_textures = false;
    bool wireframe = false;
    bool show_imgui = false;
    bool show_imgui_metrics_window = false;
    bool show_imgui_demo_window = false;
    bool show_implot_demo_window = false;
    
    #ifdef PROFILER_ENABLED
    bool show_profiler = false;
    bool paused_for_profiler = false;
    #endif

    Task_Queue task_queue;

    f64 dt;
    u32 fps = 0;

    mat4 perspective;
    mat4 orthographic;

    Assets *assets = NULL;
    Batch_Renderer *batch_renderer = NULL;
    Batch_Renderer::Per_Frame_Stats br_stats;
    Font *font = NULL;

    Texture_Array *block_textures;
    Texture_Array *item_textures;

    struct World *world = NULL;
    struct Player *player = NULL;

    GuiID active_gui = GUI_MAIN_MENU;
    struct Panel* guis[NUM_GUI_IDS];
    struct {
        struct Main_Menu *main_menu;
        struct New_World *new_world;
        struct Load_World *load_world;
        struct In_Game *in_game;
        struct Pause *pause;
        struct Settings *settings;
    } gui;
    Array<GuiID> gui_stack;

    void scroll_callback(f64 x, f64 y);
    void mouse_pos_callback(f64 x, f64 y);
    void mouse_button_callback(s32 button, s32 action, s32 mods);
    void key_callback(s32 key, s32 scancode, s32 action, s32 mods);
    void char_callback(u32 cp);
    void window_size_callback(s32 width, s32 height);
    bool mouse_pressed(s32 button);
    s32 run(s32 argc, cstr *argv);
    
    void set_gui(GuiID id);
    void push_gui();
    void pop_gui();

    void load_world(str name);
    void unload_world();
    bool world_exists(str name);

    void update_matrices_and_frustum();

private:
    bool init_glfw();
    bool init_glew();
    bool init_gl();
    bool init();
    void register_textures();
    void generate_item_block_textures();
    void check_resize();
    void check_fullscreen();
    void shutdown();
    void update();
    void render();
    #ifndef IMGUI_DISABLE
    void draw_imgui();
    #endif
};


extern Game *game;


#endif