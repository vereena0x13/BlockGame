#define SHADER_HOT_RELOAD

enum {
    VERTEX_SHADER   = 1,
    FRAGMENT_SHADER = 2,
    GEOMETRY_SHADER = 4
};

#define SHADER_TYPES(X)                          \
    X(VERTEX_SHADER,   vert, GL_VERTEX_SHADER  ) \
    X(GEOMETRY_SHADER, geom, GL_GEOMETRY_SHADER) \
    X(FRAGMENT_SHADER, frag, GL_FRAGMENT_SHADER)


struct Shader {
    rstr name;
    u8 flags;
    bool valid = false;

    Shader(rstr _name, u8 _flags) : name(_name), flags(_flags) {
        reload();
    }

    bool reload() {
        unbind();

        auto old_program = program;
        program = glCreateProgram();
        
        char buffer[1024];

        #define X(t, n, s)                                              \
            GLuint n##_id;                                              \
            if(flags & t) {                                             \
                sprintf(buffer, "assets/shaders/%s/%s.glsl", name, #n); \
                if(!attach_shader(buffer, s, n##_id)) {                 \
                    glDeleteProgram(program);                           \
                    return false;                                       \
                }                                                       \
            }
        SHADER_TYPES(X)
        #undef X

        GLint success;
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint info_log_length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

            GLchar *info_log = cast(GLchar*, talloc(sizeof(GLchar) * (info_log_length + 1)));
            assert(info_log); // NOTE: Yes I'm being lazy, fite me.
            glGetProgramInfoLog(program, info_log_length, 0, info_log);

            log(ERROR, "%s\n", info_log);
            glDeleteProgram(program);
            program = old_program;
            return false;
        }

        #define X(t, n, s)                       \
            if(flags & t) {                      \
                glDetachShader(program, n##_id); \
                glDeleteShader(n##_id);          \
            }
        SHADER_TYPES(X)
        #undef X

        if(valid) glDeleteProgram(old_program);

        valid = true;
        return true;
    }

    void deinit() {
        glDeleteProgram(program);
    }

    void bind() {
        glUseProgram(program);
    }

    void unbind() {
        glUseProgram(0);
    }

    GLint get_uniform_location(cstr name) {
        return glGetUniformLocation(program, name);
    }

    GLint get_uniform_location(rstr name) {
        return get_uniform_location(cast(cstr, name));
    }

    void set_uniform_1iv(GLint location, GLsizei count, GLint const* values) {
        glProgramUniform1iv(program, location, count, values);
    }

    void set_uniform_1i(GLint location, GLint value) {
        glProgramUniform1i(program, location, value);
    }

    void set_uniform_vec3(GLint location, vec3 value) {
        glProgramUniform3f(program, location, value.x, value.y, value.z);
    }

    void set_uniform_vec4(GLint location, vec4 value) {
        glProgramUniform4f(program, location, value.x, value.y, value.z, value.w);
    }

    void set_uniform_mat4(GLint location, mat4 matrix) {
        glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, matrix.value_ptr());
    }

private:
    GLuint program;

    bool attach_shader(cstr name, GLenum type, GLuint& r) {
        str src = read_entire_file(name);

        if(!src) {
            tfprintf(stderr, "File not found: %s\n", name);
            exit(1);
        }

        auto id = glCreateShader(type);
        glShaderSource(id, 1, (char const* const*) &src, 0);
        glCompileShader(id);

        xfree(strhdr(src));

        GLint success;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint info_log_length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);

            GLchar *info_log = cast(GLchar*, talloc(sizeof(GLchar) * (info_log_length + 1)));
            assert(info_log); // NOTE: Yes I'm being lazy, fite me.
            glGetShaderInfoLog(id, info_log_length, 0, info_log);

            log(ERROR, "Error compiling shader `%s`:\n%s\n", name, info_log);

            glDeleteShader(id);
            return false;
        }

        glAttachShader(program, id);
        r = id;
        return true;
    }
};


/*

Idea for an improved Shader API:

struct MyShader : public Shader {
    GLint u_proj;
    GLint u_view;

    MyShader() : Shader("myshader") {
        uniform_mat4("u_proj", u_proj);
        uniform_mat4("u_view", u_view);
    }

    void reload() override {
        Shader::reload();
        // Shader::reload assigns all registered uniform variables
        // to their location via get_uniform_location 
    }
};

*/