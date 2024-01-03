struct Block_Furnace : public Block {
    struct Entity : public Block_Entity {
        Direction facing = UNKNOWN;
        bool active = false;

        void serialize(ByteBuf *bb) override {
            Block_Entity::serialize(bb);
            bb->write_u8(facing);
        }

        void deserialize(ByteBuf *bb) override {
            Block_Entity::deserialize(bb);
            facing = cast(Direction, bb->read_u8());
        }
    };

    s32 tex_side;
    s32 tex_top;
    s32 tex_front_on;
    s32 tex_front_off;

    Block_Furnace(blkid id) : Block(id) {}

    void register_textures(Texture_Array *itexs) override {
        tex_side = itexs->add_from_file("blocks/furnace_side");
        tex_top = itexs->add_from_file("blocks/furnace_top");
        tex_front_on = itexs->add_from_file("blocks/furnace_front_on");
        tex_front_off = itexs->add_from_file("blocks/furnace_front_off");
    }

    s32 get_texture(World *world, vec3i pos, Direction face) override {
        if(face == UP) return tex_top;

        if(world) {
            auto be = dynamic_cast<Entity*>(world->get_block_entity(pos));
            assert(be != NULL);
            if(face == be->facing) return be->active ? tex_front_on : tex_front_off;
        } else if(face == EAST) {
            return tex_front_off;
        }
        
        return tex_side;
    }

    Block_Entity* create_block_entity(World *world, vec3i pos) override {
        return xnew(Entity);
    }

    void on_placed(World *world, vec3i pos, struct Hit *hit, f32 player_yaw) override {
        auto be = dynamic_cast<Entity*>(world->get_block_entity(pos));
        assert(be != NULL);
        be->facing = DIRECTION_OPPOSITE[angle_to_direction(player_yaw)];
    }
};