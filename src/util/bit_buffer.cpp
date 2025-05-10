#define BIT_BUFFER_SHARED                                           \
    void free() {                                                   \
        xfree(data, a);                                             \
    }                                                               \
                                                                    \
    u32 get(u64 index) {                                            \
        assert(index < entries);                                    \
                                                                    \
        u64 ia = index / entries_per_u64;                           \
        u64 ie = (index - ia * entries_per_u64) * bits_per_entry;   \
        return (data[ia] >> ie) & mask;                             \
    }                                                               \
                                                                    \
    void set(u64 index, u32 value) {                                \
        assert(index < entries);                                    \
        assert((value & mask) == value);                            \
                                                                    \
        u64 ia = index / entries_per_u64;                           \
        u64 ie = (index - ia * entries_per_u64) * bits_per_entry;   \
        u64 m = ~(mask << ie);                                      \
        data[ia] = (data[ia] & m) | (value << ie);                  \
    }



struct Bit_Buffer {
    u8 bits_per_entry;
    u8 entries_per_u64;
    u64 entries;
    u64 mask;
    u64 *data;
    Allocator *a;

    
    Bit_Buffer(u8 _bits_per_entry, u64 _entries, Allocator *_a = ::allocator) {
        bits_per_entry  = _bits_per_entry;
        entries_per_u64 = 64 / bits_per_entry;
        entries         = _entries;
        mask            = (1 << bits_per_entry) - 1;
        a               = _a;

        assert(bits_per_entry >= 1 && bits_per_entry <= 64); // TODO?
        assert(entries <= (1 << bits_per_entry));
        
        u64 n = (entries + entries_per_u64 - 1) / entries_per_u64;
        data = cast(u64*, xalloc(sizeof(64) * n, a));
    }

    BIT_BUFFER_SHARED
};



template<u64 _bits_per_entry>
struct Static_Bit_Buffer {
    static constexpr u64 bits_per_entry = _bits_per_entry;
    static constexpr u64 max_entries    = 1 << bits_per_entry;
    static constexpr u64 mask           = (1 << bits_per_entry) - 1;

    static_assert(bits_per_entry >= 1 && bits_per_entry <= 64);


    u8 entries_per_u64;
    u64 entries;
    u64 *data;
    Allocator *a;


    Static_Bit_Buffer(u64 _entries, Allocator *_a = ::allocator) {
        entries_per_u64 = 64 / bits_per_entry;
        entries         = _entries;
        a               = _a;

        assert(entries <= max_entries);

        u64 n = (entries + entries_per_u64 - 1) / entries_per_u64;
        data = cast(u64*, xalloc(sizeof(64) * n, a));
    }

    BIT_BUFFER_SHARED
};



#undef BIT_BUFFER_SHARED