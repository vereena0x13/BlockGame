struct Paletted_Bit_Buffer {
    Bit_Buffer *data;
    Array<u32> palette;
    u32 max_entries;

    Paletted_Bit_Buffer(u64 entries) {
        data = xnew(Bit_Buffer, 4, entries);
        palette.resize(16);
        max_entries = 16;
    }

    void free() {
        data->free();
        xfree(data);

        palette.free();
    }

    u32 get(u64 index) {
        return palette[data->get(index)];
    }

    void set(u64 index, u32 value) {
        u64 ip = palette.index(value);
        if(ip == -1) {
            if(palette.count + 1 > max_entries) resize();
            ip = palette.count;
            palette.push(value);
        }
        data->set(index, ip);
    }

private:
    void resize() {
        auto new_data = xnew(Bit_Buffer, data->bits_per_entry + 1, data->entries);
        max_entries <<= 1;

        for(u64 i = 0; i < data->entries; i++) {
            new_data->set(i, palette[data->get(i)]);
        }
        
        data->free();
        xfree(data);

        data = new_data;
    }
};