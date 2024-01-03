f32 const MOVE_SPEED        = 0.1f;
f32 const MOVE_SPEED_SLOW   = 0.05f;
f32 const MOUSE_SENSITIVITY = 0.15f;

struct Camera {
    Game *game;
    Entity *ent;
    f32 pitch;
    f32 yaw;
    quat rot;
    bool cursor_grabbed;

    Camera(Game *_game, Entity *_ent) : 
        game(_game),
        ent(_ent),
        pitch(0),
        yaw(0),
        cursor_grabbed(false)
    {
    }

    mat4 view() {
        return mat4::translate(-ent->pos.x, -ent->pos.y, -ent->pos.z) * rot.matrix();
    }

    vec3 dir() {
        return conj(rot, vec3(0,0,-1));
    }

    void update() {
        if(!cursor_grabbed) return;
        
        rot = quat::rotate(-yaw, vec3(0, -1, 0)) * 
              quat::rotate(-pitch, vec3(-1, 0, 0));
    }

    void mouse_moved(f32 dx, f32 dy) {
        if(!cursor_grabbed) return;

        yaw += dx * MOUSE_SENSITIVITY;
        if(yaw < 0) yaw += 360;
        if(yaw > 360) yaw -= 360;

        pitch += dy * MOUSE_SENSITIVITY;
        pitch = clamp(pitch, -89.0f, 89.0f);
    }

    void grab_cursor() {
        if(cursor_grabbed) return;
        cursor_grabbed = true;
        glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void release_cursor() {
        if(!cursor_grabbed) return;
        cursor_grabbed = false;
        glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void serialize(ByteBuf *bb) {
        bb->write_f32(pitch);
        bb->write_f32(yaw);
    }

    void deserialize(ByteBuf *bb) {
        pitch = bb->read_f32();
        yaw = bb->read_f32();
    }
};