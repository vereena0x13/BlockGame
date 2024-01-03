struct Throttle {
    u32 count = 0;
    u32 max;

    Throttle(u32 _max) : max(_max) {}

    bool act() {
        if(count >= max) return false;
        count++;
        return true;
    }

    void reset() {
        count = 0;
    }
};

struct Throttles {
    Throttle chunk_mesh_gen = Throttle(4);
    Throttle region_save = Throttle(4);

    void reset() {
        chunk_mesh_gen.reset();
        region_save.reset();
    }
};

Throttles throttles;