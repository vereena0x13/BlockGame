struct Entity {
    Game *game;

    World *world;
    vec3 pos;

    Entity(Game *_game) : game(_game), pos(vec3(0, 0, 0)) {
    }

    virtual void deinit() {
        
    }

    virtual void update() {

    }

    virtual void draw() {

    }

    virtual void serialize(ByteBuf *bb) {
        pos.write(bb);
    }

    virtual void deserialize(ByteBuf *bb) {
        pos = vec3::read(bb);
    }
};