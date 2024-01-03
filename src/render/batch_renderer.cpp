// TODO: Support Texture Arrays!!!

// TODO: Implement a clipping rectangle and
// a clip rect stack.

// NOTE: Batch_Renderer is a 3D quad renderer that batches quads into draw calls
// automatically, handling textures as well. It provides convenience methods for
// 2D drawing with z = 0.
struct Batch_Renderer {
    #pragma pack(push, 1)
    struct Vertex {
        vec3 pos;
        vec4 color;
        vec2 uv;
        s32 tex;
    };
    #pragma pack(pop)

    struct Per_Frame_Stats {
        u32 quads;
        u32 vertices;
        u32 indices;
        u32 draw_calls;
    };

    static constexpr u32 MAX_VERTICES = 1024 * 128;
    static constexpr u32 MAX_INDICES = MAX_VERTICES * 3;
    static constexpr u32 MAX_TEXTURE_SLOTS = 16; // TODO

private:
    Shader *shader;
    GLint u_textures;
    GLint u_proj;
    GLint u_view;

    GLuint white_texture;
    Slot_Allocator<GLuint, MAX_TEXTURE_SLOTS> textures;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;

    Static_Array<Vertex, MAX_VERTICES> vertices;
    Static_Array<u32, MAX_INDICES> indices;

    Per_Frame_Stats per_frame_stats;

public:
    void init() {
        shader = xnew(Shader, "batch", VERTEX_SHADER | FRAGMENT_SHADER);
        u_textures = shader->get_uniform_location("u_textures");
        u_proj = shader->get_uniform_location("u_proj");
        u_view = shader->get_uniform_location("u_view");


        s32 samplers[MAX_TEXTURE_SLOTS];
        for(s32 i = 0; i < MAX_TEXTURE_SLOTS; i++) 
            samplers[i] = i;
        shader->set_uniform_1iv(u_textures, MAX_TEXTURE_SLOTS, samplers);


        glCreateTextures(GL_TEXTURE_2D, 1, &white_texture);
        glTextureStorage2D(white_texture, 1, GL_RGBA8, 1, 1);
        u32 white = 0xFFFFFFFF;
        glTextureSubImage2D(white_texture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &white);
        glTextureParameteri(white_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(white_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(white_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(white_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);


        vbo.init(sizeof(vertices.data), GL_DYNAMIC_DRAW);
        ibo.init(sizeof(indices.data), GL_DYNAMIC_DRAW);
        
        vao.init();
        vao.add_vertex_buffer(
            vbo,
            Vertex_Element(GL_FLOAT, 3),
            Vertex_Element(GL_FLOAT, 4),
            Vertex_Element(GL_FLOAT, 2),
            Vertex_Element(GL_INT, 1)
        );
        vao.set_index_buffer(ibo);
    }

    void deinit() {
        shader->deinit();
        xfree(shader);

        glDeleteTextures(1, &white_texture);
        vao.deinit();
        vbo.deinit();
        ibo.deinit();
    }

    void set_projection(mat4 const& proj) {
        shader->set_uniform_mat4(u_proj, proj);
    }

    void set_view(mat4 const& view) {
        shader->set_uniform_mat4(u_view, view);
    }

    void flush() {
        TIMED_FUNCTION();
        
        if(vertices.count == 0) return;

        assert(indices.count % 3 == 0 && indices.count > 0);

        per_frame_stats.vertices += vertices.count;
        per_frame_stats.indices += indices.count;
        per_frame_stats.draw_calls++;

        vbo.set_subdata(&vertices.data, vertices.count * sizeof(Vertex));
        ibo.set_subdata(&indices.data, indices.count * sizeof(u32));

        shader->bind();
        vao.bind();

        for(u32 i = 0; i < textures.count; i++)
           glBindTextureUnit(i, textures.slots[i]);

        glDrawElements(GL_TRIANGLES, indices.count, GL_UNSIGNED_INT, 0);

        for(u32 i = 0; i < textures.count; i++)
            glBindTextureUnit(i, 0);

        vao.unbind();
        shader->unbind();

        begin();
    }

    void ensure_available(u32 v, u32 i) {
        assert(v < MAX_VERTICES);
        assert(i < MAX_INDICES);
        if(vertices.count + v > MAX_VERTICES || indices.count + i > MAX_INDICES) flush();
    }

    void begin(mat4 const& view_matrix) {
        set_view(view_matrix);
        begin();
    }

    void begin() {
        vertices.clear();
        indices.clear();
        textures.clear();
    }

    void end() {
        if(vertices.count > 0) flush();
    }

    Per_Frame_Stats end_frame() {
        end();
        auto stats = per_frame_stats;
        memset(&per_frame_stats, 0, sizeof(Per_Frame_Stats));
        return stats;
    }

    void push_quad(vec3 vtl, vec3 vtr, vec3 vbr, vec3 vbl, vec4 color, vec2 uvs[2], GLuint texture) {
        ensure_available(4, 6);

        s32 tex_index;
        if(glIsTexture(texture)) tex_index = textures.alloc(texture);
        else                     tex_index = textures.alloc(white_texture);
        assert(tex_index != -1);

        auto apm_color = alpha_premultiply(color);
        u32 tl = vertices.push({ vtl, apm_color, uvs[0], tex_index });
        u32 tr = vertices.push({ vtr, apm_color, uvs[1], tex_index });
        u32 br = vertices.push({ vbr, apm_color, uvs[2], tex_index });
        u32 bl = vertices.push({ vbl, apm_color, uvs[3], tex_index });

        indices.push(tl);
        indices.push(tr);
        indices.push(br);
        indices.push(br);
        indices.push(bl);
        indices.push(tl);

        per_frame_stats.quads++;
    }

    void push_quad(vec3 vtl, vec3 vtr, vec3 vbr, vec3 vbl, vec4 color, GLuint texture) {
        vec2 uvs[] = {
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f)
        };
        push_quad(vtl, vtr, vbr, vbl, color, uvs, texture);
    }

    void push_solid_quad(vec3 vtl, vec3 vtr, vec3 vbr, vec3 vbl, vec4 color) {
        push_quad(vtl, vtr, vbr, vbl, color, white_texture);
    }

    void push_textured_quad(vec3 vtl, vec3 vtr, vec3 vbr, vec3 vbl, GLuint texture) {
        push_quad(vtl, vtr, vbr, vbl, vec4(1,1,1,1), texture);
    }

    void push_textured_quad(vec3 vtl, vec3 vtr, vec3 vbr, vec3 vbl, Texture* texture) {
        push_quad(vtl, vtr, vbr, vbl, vec4(1,1,1,1), texture->id);
    }

    void push_quad(f32 x, f32 y, f32 w, f32 h, vec4 color, vec2 uvs[4], GLuint texture) {
        push_quad(
            vec3(x, y, 0),
            vec3(x + w, y, 0),
            vec3(x + w, y + h, 0),
            vec3(x, y + h, 0),
            color,
            uvs,
            texture
        );
    }

    void push_quad(f32 x, f32 y, f32 w, f32 h, vec4 color, GLuint texture) {
        vec2 uvs[] = {
            vec2(0.0f, 0.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 1.0f),
            vec2(0.0f, 1.0f)
        };
        push_quad(x, y, w, h, color, uvs, texture);
    }

    void push_solid_quad(f32 x, f32 y, f32 w, f32 h, vec4 color) {
        push_quad(x, y, w, h, color, white_texture);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, vec4 color, GLuint texture) {
        push_quad(x, y, w, h, color, texture);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, GLuint texture) {
        push_quad(x, y, w, h, vec4(1.0f, 1.0f, 1.0f, 1.0f), texture);
    }

    void push_textured_quad(f32 x, f32 y, f32 w, f32 h, Texture *texture) {
        push_quad(x, y, w, h, vec4(1.0f, 1.0f, 1.0f, 1.0f), texture->id);
    }

    void push_solid_quad(vec2 pos, vec2 size, vec4 color) {
        push_quad(pos.x, pos.y, size.x, size.y, color, white_texture);
    }

    void push_textured_quad(vec2 pos, vec2 size, vec4 color, GLuint texture) {
        push_quad(pos.x, pos.y, size.x, size.y, color, texture);
    }

    void push_textured_quad(vec2 pos, vec2 size, vec4 color, Texture *texture) {
        push_quad(pos.x, pos.y, size.x, size.y, color, texture->id);
    }

    void push_textured_quad(vec2 pos, vec2 size, GLuint texture) {
        push_quad(pos.x, pos.y, size.x, size.y, vec4(1.0f, 1.0f, 1.0f, 1.0f), texture);
    }

    void push_textured_quad(vec2 pos, vec2 size, Texture *texture) {
        push_quad(pos.x, pos.y, size.x, size.y, vec4(1.0f, 1.0f, 1.0f, 1.0f), texture->id);
    }
};