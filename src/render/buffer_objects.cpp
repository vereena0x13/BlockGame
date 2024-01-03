template<GLenum type>
struct GL_Buffer {
    GLuint id;

    void init() {
        glCreateBuffers(1, &id);
    }

    void init(u64 size, GLenum usage) {
        init();
        glNamedBufferData(id, size, nullptr, usage);
    }

    void init(void const* data, u64 size, GLenum usage) {
        init();
        glNamedBufferData(id, size, data, usage);
    }

    void deinit() {
        glDeleteBuffers(1, &id);
    }

    void bind() {
        glBindBuffer(type, id);
    }

    void unbind() {
        glBindBuffer(type, 0);
    }

    void set_data(void const* data, u32 size, GLenum usage) {
        glNamedBufferData(id, size, data, usage);
    }

    void set_subdata(void const* data, u32 size, u32 offset = 0) {
        glNamedBufferSubData(id, offset, size, data);
    }

    void* map(GLintptr offset, GLsizeiptr length, GLbitfield access) {
        return glMapNamedBufferRange(id, offset, length, access);
    }

    void unmap() {
        glUnmapNamedBuffer(id);
    }
};

using Vertex_Buffer = GL_Buffer<GL_ARRAY_BUFFER>;
using Index_Buffer = GL_Buffer<GL_ELEMENT_ARRAY_BUFFER>;

struct Vertex_Element {
    GLenum type;
    s32 count;
    bool normalized;

    Vertex_Element(GLenum _type, s32 _count, bool _normalized = false) : type(_type), count(_count), normalized(_normalized) {}

    s64 size() const {
        switch(type) {
            case GL_FLOAT:
            case GL_INT:
            case GL_UNSIGNED_INT:
                return count * 4;
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
                return count;
        }
        
        assert(0);
        return 0;
    }
};

struct Vertex_Array {
    GLuint id;
    GLuint binding_index;

    void init() {
        glCreateVertexArrays(1, &id);
        binding_index = 0;
    }

    void deinit() {
        glDeleteVertexArrays(1, &id);
    }

    void bind() {
        glBindVertexArray(id);
    }

    void unbind() {
        glBindVertexArray(0);
    }

    template<typename... T>
    GLuint add_vertex_buffer(Vertex_Buffer const& vbo, T... _format) {
        static_assert((otr::type_eq<T, Vertex_Element> && ...));
        
        auto n = sizeof...(_format);
        Vertex_Element format[] = { _format... };

        s64 stride = 0;
        for(u32 i = 0; i < n; i++) {
            stride += format[i].size();
        }

        glVertexArrayVertexBuffer(id, binding_index, vbo.id, 0, stride);

        s64 offset = 0;
        for(u32 i = 0; i < n; i++) {
            glEnableVertexArrayAttrib(id, i);
            glVertexArrayAttribBinding(id, i, binding_index);

            auto const& e = format[i];
            switch(e.type) {
                case GL_FLOAT:
                    assert(!e.normalized);
                    glVertexArrayAttribFormat(id, i, e.count, e.type, GL_FALSE, offset);
                    break;
                case GL_BYTE:
                case GL_UNSIGNED_BYTE:
                case GL_INT:
                case GL_UNSIGNED_INT:
                    if(e.normalized) {
                        glVertexArrayAttribFormat(id, i, e.count, e.type, GL_TRUE, offset);
                    } else {
                        glVertexArrayAttribIFormat(id, i, e.count, e.type, offset);
                    }
                    break;
                default:
                    assert(0);
            }
            
            offset += e.size();
        }

        return binding_index++;
    }

    void set_index_buffer(Index_Buffer const& ibo) {
        glVertexArrayElementBuffer(id, ibo.id);
    }
};

// TODO: struct Render_Buffer ...

struct Frame_Buffer {
    GLuint id;

    void init() {
        glCreateFramebuffers(1, &id);
    }

    void deinit() {
        glDeleteFramebuffers(1, &id);
    }

    void bind(GLenum target = GL_FRAMEBUFFER) {
        glBindFramebuffer(target, id);
    }

    void unbind(GLenum target = GL_FRAMEBUFFER) {
        glBindFramebuffer(target, 0);
    }

    void attach_draw_buffers(GLsizei n, GLenum const* xs) {
        glNamedFramebufferDrawBuffers(id, n, xs);
    }

    void attach_texture(GLenum attachment, GLuint texture, GLint level = 0) {
        glNamedFramebufferTexture(id, attachment, texture, level);
    }

    void attach_texture_layer(GLenum attachment, GLuint texture, GLint layer, GLint level = 0) {
        glNamedFramebufferTextureLayer(id, attachment, texture, level, layer);
    }

    GLenum status(GLenum target = GL_FRAMEBUFFER) {
        return glCheckNamedFramebufferStatus(id, target);
    }

    void blit(GLuint dest, GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1, GLint dy1, GLbitfield mask, GLenum filter) {
        glBlitNamedFramebuffer(id, dest, sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter);
    }
};