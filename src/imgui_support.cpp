void imgui_init(GLFWwindow *window) {
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // NOTE TODO: We'll need this for hot code reloading!
    // ImGui::SetAllocatorFunctions(alloc_fn, free_fn, user_data);

    char glsl_version_string[13];
    assert(snprintf(glsl_version_string, sizeof(glsl_version_string)/sizeof(char), "#version %d%d0", GL_MAJOR, GL_MINOR) == 12);
    ImGui_ImplOpenGL3_Init(glsl_version_string);

    ImGuiIO& io = ImGui::GetIO();
    //io.IniFilename = nullptr;
    auto fc = ImFontConfig();
    io.Fonts->AddFontFromFileTTF("assets/fonts/Meslo LG L Regular Nerd Font Complete.ttf", 20, &fc);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();
    ImPlot::GetStyle().AntiAliasedLines = true;
}

void imgui_shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void imgui_begin_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

namespace ImGui {
    void HelpMarker(const char* desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}