struct Sun {
    Shader *shader;
    GLint u_proj;
    GLint u_view;
    GLint u_model;
    GLint u_tex;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;

    Texture *sun_tex;

    vec3 view_pos;
    f32 angle = 0.0f;

    struct Vtx {
        vec3 pos;
        vec2 uv;
    };

    Sun() {
        shader = xnew(Shader, "sun", VERTEX_SHADER | FRAGMENT_SHADER);
        u_proj = shader->get_uniform_location("u_proj");
        u_view = shader->get_uniform_location("u_view");
        u_model = shader->get_uniform_location("u_model");
        u_tex = shader->get_uniform_location("u_tex");
        shader->set_uniform_1i(u_tex, 0);

        Vtx vertices[] = {
            { vec3(-1, 0, -1), vec2(0, 0) },
            { vec3( 1, 0, -1), vec2(0, 1) },
            { vec3( 1, 0,  1), vec2(1, 1) },
            { vec3(-1, 0,  1), vec2(1, 0) }
        };

        u32 indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        vao.init();
        vbo.init(sizeof(vertices), GL_STATIC_DRAW);
        ibo.init(sizeof(indices), GL_STATIC_DRAW);

        vbo.set_subdata(vertices, sizeof(vertices));
        ibo.set_subdata(indices, sizeof(indices));

        vao.add_vertex_buffer(
            vbo,
            Vertex_Element(GL_FLOAT, 3),
            Vertex_Element(GL_FLOAT, 2)
        );
        vao.set_index_buffer(ibo);

        sun_tex = game->assets->get_texture("sun");
    }

    void deinit() {
        shader->deinit();
        xfree(shader);

        vbo.deinit();
        ibo.deinit();
        vao.deinit();
    }

    void set_projection(mat4 proj) {
        shader->set_uniform_mat4(u_proj, proj);
    }

    void set_view(mat4 view) {
        shader->set_uniform_mat4(u_view, view);
    }

    void set_view_pos(vec3 view_pos) {
        this->view_pos = view_pos;
    }

    void draw() {
        angle += 0.1f;
        if(angle >= 360) {
            angle -= 360;
        }

        f32 sun_size = 35;
        mat4 model = mat4::scale(sun_size,sun_size,sun_size) * 
                     mat4::translate(0, 1000, 0) * 
                     quat::rotate(angle, vec3(0, 0, 1)).matrix();

        shader->bind();
        shader->set_uniform_mat4(u_model, model);
        vao.bind();
        sun_tex->bind(0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        vao.unbind();
        shader->unbind();
    }
};

struct Sky {
    Shader *shader;
    GLint u_proj;
    GLint u_view;
    GLint u_view_pos;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;

    vec3 color;

    Sun sun;

    struct Vertex {
        vec4 pos;
    };

    Sky(vec3 _color) : color(_color) {
        shader = xnew(Shader, "sky", VERTEX_SHADER | FRAGMENT_SHADER);
        u_proj = shader->get_uniform_location("u_proj");
        u_view = shader->get_uniform_location("u_view");
        u_view_pos = shader->get_uniform_location("u_view_pos");


        Vertex vertices[] = {
            vec4(-1, -1, 0, 1),
            vec4(1, -1, 0, 1), 
            vec4(1, 1, 0, 1), 
            vec4(-1, 1, 0, 1)
        };

        u32 indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        vao.init();
        vbo.init(sizeof(vertices), GL_STATIC_DRAW);
        ibo.init(sizeof(indices), GL_STATIC_DRAW);

        vbo.set_subdata(vertices, sizeof(vertices));
        ibo.set_subdata(indices, sizeof(indices));

        vao.add_vertex_buffer(
            vbo,
            Vertex_Element(GL_FLOAT, 4)
        );

        vao.set_index_buffer(ibo);
    }

    void deinit() {
        shader->deinit();
        xfree(shader);

        vbo.deinit();
        ibo.deinit();
        vao.deinit();

        sun.deinit();
    }

    #ifdef SHADER_HOT_RELOAD
    void reload_shader() {
        if(shader->reload()) {
            u_proj = shader->get_uniform_location("u_proj");
            u_view = shader->get_uniform_location("u_view");
            u_view_pos = shader->get_uniform_location("u_view_pos");
        } else {
            log(ERROR, "Failed to reload sky shader");
        }
    }
    #endif

    void set_projection(mat4 proj) {
        shader->set_uniform_mat4(u_proj, proj);
        sun.set_projection(proj);
    }

    void set_view(mat4 view) {
        shader->set_uniform_mat4(u_view, view);
        sun.set_view(view);
    }

    void set_view_pos(vec3 view_pos) {
        shader->set_uniform_vec3(u_view_pos, view_pos);
        sun.set_view_pos(view_pos);
    }

    void draw() {
        //if(!shader->valid) return;
        //shader->bind();
        //vao.bind();
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //shader->unbind();
        //vao.unbind();

        sun.draw();
    }

    void update() {

    }
};
