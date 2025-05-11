u32 const CONTAINER_DEFAULT_SLOT_LIMIT = 64;

enum Container_Flags : u8 {
    CONTAINER_FLAG_NONE              = 0,
    CONTAINER_FLAG_NO_INSERT         = 1,
    CONTAINER_FLAG_NO_EXTRACT        = 2,
    CONTAINER_FLAG_FILTER_INSERTIONS = 4
};

struct Container {
    Container_Flags flags;
    Item_Stack *slots;
    u32 size;
    u32 slot_limit = CONTAINER_DEFAULT_SLOT_LIMIT;

    // NOTE: Static_Bitset may not be the best way to implement this;
    // we're using up 32 bytes for something that isn't used extremely
    // often (?). But for now, blegh, who cares. Profiling will tell me
    // if this needs to change.
    //              - vereena, 5/13/20
    // NOTE: 4 words * 64 bits per word = 256 bits
    Static_Bitset<4> insertion_filter;

    Container(u32 _size, Container_Flags _flags = CONTAINER_FLAG_NONE) : size(_size), flags(_flags) {
        u64 n = sizeof(Item_Stack) * size;
        slots = cast(Item_Stack*, xalloc(n));
        memset(slots, 0, sizeof(n));
    }

    void deinit() {
        xfree(slots);
    }

    bool accepts(Item *item) {
        if(flags & CONTAINER_FLAG_NO_INSERT) return false;
        if(flags & CONTAINER_FLAG_FILTER_INSERTIONS) return insertion_filter.get(item->id);
        return true;
    }

    u32 count(Item *item) {
        u32 c = 0;
        for(u32 i = 0; i < size; i++) {
            auto const& slot = slots[i];
            if(slot.item == item && slot.count) {
                c += slot.count;
            }
        }
        return c;
    }

    u32 total_count() {
        u32 c = 0;
        for(u32 i = 0; i < size; i++) {
            auto const& slot = slots[i];
            if(slot.item && slot.count) c += slot.count;
        }
        return c;
    }

    bool contains(Item *item) {
        if(!accepts(item)) return false;
        for(u32 i = 0; i < size; i++) {
            auto const& slot = slots[i];
            if(slot.item == item && slot.count) return true;
        }
        return false;
    }

    u32 insert(Item_Stack const& stack, u32 slot_index) {
        if(!accepts(stack.item)) return 0;

        assert(slot_index < size);
        auto& slot = slots[slot_index];
        
        if(slot.count == 0) { 
            u32 n = min(stack.count, slot_limit);
            slot.item = stack.item;
            slot.count = n;
            return n;
        } else if(slot.item == stack.item) {
            u32 n = min(stack.count, slot_limit - slot.count);
            slot.count += n;
            return n;
        }

        return 0;
    }

    u32 insert(Item_Stack const& stack) {
        if(!accepts(stack.item)) return 0;

        u32 left = stack.count;
        u32 r = 0;
        
        for(u32 i = 0; i < size && left > 0; i++) {
            auto& slot = slots[i];
            if(slot.item == stack.item && slot.count < slot_limit) {
                u32 n = min(left, slot_limit - slot.count);
                slot.count += n;
                r += n;
                left -= n;
            }
        }

        for(u32 i = 0; i < size && left > 0; i++) {
            auto& slot = slots[i];
            if(slot.count == 0) {
                u32 n = min(left, slot_limit);
                slot.count = n;
                slot.item = stack.item;
                r += n;
                left -= n;
            }
        }

        return r;
    }

    Item_Stack remove(u32 slot_index) {
        assert(slot_index < size);
        auto& slot = slots[slot_index];

        if(!slot.is_valid()) return Item_Stack();

        auto stack = slot;
        slot = Item_Stack();
        return stack;
    }

    Item_Stack remove(u32 slot_index, u32 count) {
        assert(slot_index < size);
        auto& slot = slots[slot_index];
        
        if(!slot.is_valid()) return Item_Stack();
        
        auto item = slot.item;
        u32 n = min(count, slot.count);

        slot.count -= n;
        if(slot.count == 0) slot.item = NULL;

        return Item_Stack(item, n);
    }

    void serialize(ByteBuf *bb) {
        bb->write_u8(flags);

        u64 count_index = bb->reserve(4);
        u32 count = 0;
        for(u32 i = 0; i < size; i++) {
            auto& slot = slots[i];
            if(slot.item && slot.count) {
                count++;
                bb->write_u32(i);
                slot.serialize(bb);
            }
        }
        bb->set_u32(count_index, count);
        
        if(flags & CONTAINER_FLAG_FILTER_INSERTIONS) {
            insertion_filter.serialize(bb);
        }
    }

    void deserialize(ByteBuf *bb) {
        flags = cast(Container_Flags, bb->read_u8());

        u32 count = bb->read_u32();
        for(u32 i = 0; i < count; i++) {
            u32 slot = bb->read_u32();
            slots[slot] = Item_Stack::deserialize(bb);
        }

        if(flags & CONTAINER_FLAG_FILTER_INSERTIONS) {
            insertion_filter.deserialize(bb);
        }
    }
};