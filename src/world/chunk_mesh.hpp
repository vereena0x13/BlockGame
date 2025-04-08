enum Chunk_Mesh_Sort_Type {
    PARTIAL,
    FULL
};

enum Chunk_Mesh_Part {
    OPAQUE,
    TRANSPARENT
};

u32 const CHUNK_N_FACES = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6;
u32 const CHUNK_N_VERTICES = CHUNK_N_FACES * 4;
u32 const CHUNK_N_INDICES = CHUNK_N_FACES * 6;

// TODO: this could be much more efficient / performant
//         - Vtx::pos doesn't need to be a vec3; we really only need
//           a few bits for any given vertex x, y, or z component
//         - greedy meshing (or, perhaps, "binary meshing"?)
//         - glDrawElementsIndirect?
//       there's definitely more that can be done but, yeah. add
//       ideas as we think of them ig :P
//                      - vereena0x13, 4-8-25

struct Chunk_Mesh {
    struct Vtx {
        vec3 pos;
        vec2 uv;
        s32 tex;
        f32 brightness;

        static void configure_vao(Vertex_Array& vao, Vertex_Buffer const& vbo, Index_Buffer const& ibo) {
            vao.add_vertex_buffer(
                vbo,
                Vertex_Element(GL_FLOAT, 3),
                Vertex_Element(GL_FLOAT, 2),
                Vertex_Element(GL_INT, 1),
                Vertex_Element(GL_FLOAT, 1)
            );
            vao.set_index_buffer(ibo);
        }
    };

    struct Face {
        vec3 pos;
        f32 dist;
        u32 index;
    };

    World *world;
    Chunk *chunk;

    Vertex_Array vao;
    Vertex_Buffer vbo;
    Index_Buffer ibo;
    
    u32 base_offset;
    u32 base_count;
    u32 trans_offset;
    u32 trans_count;

    bool dirty = true;
    bool depth_sort = false;
    bool persist = false;
    u32 index_count = 0;
    u32 *p_indices = NULL;
    Face *p_faces = NULL;

    Chunk_Mesh(Chunk *_chunk);
    void deinit();

    void render();
    void upload();
    bool prepare(vec3 view_pos);
    bool draw(Chunk_Mesh_Part part);
    void set_persist(bool persist);

private:
    void sort(Chunk_Mesh_Sort_Type type, vec3 view_pos, bool use_static_indices);
    void render_face(Block *block, vec3i block_pos, Direction dir, AABB const& bb);
    void render_block(Block *block, vec3i pos);

    static Static_Array<Vtx, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*4*6> vertices;
    static Static_Array<u32, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*6*6> indices;
    static Static_Array<Face, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*6> faces;
};