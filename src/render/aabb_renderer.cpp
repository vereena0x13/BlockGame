struct AABB_Renderer {
    Shader *shader;
    GLint u_proj;
    GLint u_view;
    GLint u_model;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;

    struct Vtx {
        vec3 pos;
        vec4 color;
    };

    AABB_Renderer() {
        shader = xnew(Shader, "basic", VERTEX_SHADER | FRAGMENT_SHADER);
        u_proj = shader->get_uniform_location("u_proj");
        u_view = shader->get_uniform_location("u_view");
        u_model = shader->get_uniform_location("u_model");
        shader->set_uniform_mat4(u_model, mat4());

        vbo.init(sizeof(Vtx) * 8, GL_DYNAMIC_DRAW);
        ibo.init(sizeof(CUBE_LINE_INDICES), GL_DYNAMIC_DRAW);

        ibo.set_subdata(cast(void*, CUBE_LINE_INDICES), sizeof(CUBE_LINE_INDICES));

        vao.init();
        vao.add_vertex_buffer(
            vbo,
            Vertex_Element(GL_FLOAT, 3),
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
    }

    void set_projection(mat4 proj) {
        shader->set_uniform_mat4(u_proj, proj);
    }

    void set_view(mat4 view) {
        shader->set_uniform_mat4(u_view, view);
    }

    void draw(AABB const& bb, vec4 color = vec4(1), f32 line_width = 2.0f) {
        Vtx vertices[8];
        for(u32 i = 0; i < 8; i++) {
            vertices[i] = {
                vec3(
                    CUBE_VERTICES[i].x ? bb.min.x : bb.max.x,
                    CUBE_VERTICES[i].y ? bb.min.y : bb.max.y,
                    CUBE_VERTICES[i].z ? bb.min.z : bb.max.z
                ),
                color
            };
        }
        vbo.set_subdata(vertices, sizeof(vertices));
        
        glLineWidth(line_width);

        shader->bind();
        vao.bind();
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        vao.unbind();
        shader->unbind();
    }
};