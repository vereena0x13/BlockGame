struct Compass3D {
    Shader *shader;
    GLuint u_proj;
    GLuint u_view;
    GLuint u_model;

    Vertex_Array vao;
    Vertex_Buffer vbo;

    struct Vtx {
        vec3 pos;
        vec3 color;
    };

    Compass3D() {
        shader = xnew(Shader, "basic", VERTEX_SHADER | FRAGMENT_SHADER);
        u_proj = shader->get_uniform_location("u_proj");
        u_view = shader->get_uniform_location("u_view");
        u_model = shader->get_uniform_location("u_model");
    
        Vtx vertices[6] = {
            { vec3(0, 0, 0), vec3(1, 0, 0) },
            { vec3(1, 0, 0), vec3(1, 0, 0) },
            { vec3(0, 0, 0), vec3(0, 1, 0) },
            { vec3(0, 1, 0), vec3(0, 1, 0) },
            { vec3(0, 0, 0), vec3(0, 0, 1) },
            { vec3(0, 0, -1), vec3(0, 0, 1) },
        };

        vbo.init(sizeof(vertices), GL_STATIC_DRAW);
        vbo.set_subdata(vertices, sizeof(vertices));

        vao.init();
        vao.add_vertex_buffer(
            vbo,
            Vertex_Element(GL_FLOAT, 3),
            Vertex_Element(GL_FLOAT, 3)
        );
    }

    void deinit() {
        shader->deinit();
        xfree(shader);

        vao.deinit();
        vbo.deinit();
    }

    void set_projection(mat4 proj) {
        shader->set_uniform_mat4(u_proj, proj);
    }

    void set_view(mat4 view) {
        shader->set_uniform_mat4(u_view, view);
    }

    void set_model(mat4 model) {
        shader->set_uniform_mat4(u_model, model);
    }

    void draw() {
        shader->bind();
        vao.bind();
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, 6);
        vao.unbind();
        shader->unbind();
    }
};