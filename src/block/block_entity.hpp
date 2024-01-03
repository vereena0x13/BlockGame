struct Block_Entity {
    World *world;
    vec3i pos;

    Block_Entity() {}

    virtual void deinit() {
    }

    virtual void update() {
        assert(world);
    }
    
    virtual void on_load(World *world) {
        this->world = world;
    }
    
    virtual void on_unload() {
        this->world = NULL;
    }
    
    virtual void serialize(ByteBuf *bb) {
        pos.write(bb);
    }

    virtual void deserialize(ByteBuf *bb) {
        pos = vec3i::read(bb);
    }
};