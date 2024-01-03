// NOTE TODO: We can change / WORD_BITS to >> 6 and
// % WORD_BITS to & 0x3F. But, I would kind of like to
// have tests before doing much optimization for something
// that is used to infrequently, as of now. *shrugs*
// Kind of hypocritical of me but yknow, sometimes
// it do be like that.
//              - vereena, 5/28/20

template<u32 n_words>
struct Static_Bitset {
    static constexpr u32 WORD_BITS = 64;

    u64 words[n_words];

    Static_Bitset() {
        memset(words, 0, sizeof(words));
    }

    bool get(u32 i) {
        assert(i >= 0 && i < n_words * WORD_BITS);
        
        u32 w = i / WORD_BITS;
        u32 b = i % WORD_BITS;
        return (words[w] & (1L << b)) != 0;
    }

    void set(u32 i) {
        assert(i >= 0 && i < n_words * WORD_BITS);

        u32 w = i / WORD_BITS;
        u32 b = i % WORD_BITS;
        words[w] |= (1L << b);
    }

    void clear(u32 i) {
        assert(i >= 0 && i < n_words * WORD_BITS);
        
        u32 w = i / WORD_BITS;
        u32 b = i % WORD_BITS;
        words[w] &= ~(1L << b);
    }

    void serialize(ByteBuf *bb) {
        bb->write_u32(n_words);
        for(u32 i = 0; i < n_words; i++) {
            bb->write_u64(words[i]);
        }
    }

    void deserialize(ByteBuf *bb) {
        u32 n = bb->read_u32();
        assert(n == n_words);
        for(u32 i = 0; i < n; i++) {
            words[i] = bb->read_u64();
        }
    }
};