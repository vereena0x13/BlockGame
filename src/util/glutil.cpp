void dump_gl_extensions() {
    printf("  GL_EXTENSIONS\n");
        
    s32 n_ext;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n_ext);
    
    char const** exts = (char const**) talloc(n_ext * sizeof(char const*));

    u32 max_len = 0;
    for(s32 i = 0; i < n_ext; i++) {
        char const *ext = (char const*) glGetStringi(GL_EXTENSIONS, i);

        exts[i] = ext;

        u32 len = strlen(ext);
        if(len > max_len) {
            max_len = len;
        }
    }

    constexpr u32 cols = 2;
    u32 column_length = max_len + 2;
    u32 column = 0;
    for(u32 i = 0; i < n_ext; i++) {
        char const* ext = exts[i];
        u32 padding = column_length - strlen(ext);

        printf("%s", ext);
        for(u32 j = 0; j < padding; j++) printf(" ");

        column++;
        if(column > (cols - 1)) {
            column = 0;
            printf("\n");
        }
    }

    if(column <= (cols - 1)) printf("\n");
}

void dump_gl_info() {
    // TODO: some kind of logging!!!

    log(DEBUG, "OpenGL Info:");

    GLint major, minor; 
    glGetIntegerv(GL_MAJOR_VERSION, &major); 
    glGetIntegerv(GL_MINOR_VERSION, &minor); 
    log(DEBUG, "  GL_MAJOR_VERSION              %d", major);
    log(DEBUG, "  GL_MINOR_VERSION              %d", minor);
    log(DEBUG, "  GL_VENDOR                     %s", glGetString(GL_VENDOR));
    log(DEBUG, "  GL_RENDERER                   %s", glGetString(GL_RENDERER));
    log(DEBUG, "  GL_VERSION                    %s", glGetString(GL_VERSION));
    log(DEBUG, "  GL_SHADING_LANGUAGE_VERSION   %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
}