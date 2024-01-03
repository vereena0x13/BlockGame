// TODO: https://nullprogram.com/blog/2017/10/06/

cstr utf8_to_codepoint(cstr p, u32 *dst) {
    u32 res, n;
    switch(*p & 0xF0) {
        case 0xF0: res = *p & 0x07; n = 3; break;
        case 0xE0: res = *p & 0x0F; n = 2; break;
        case 0xD0:
        case 0xC0: res = *p & 0x1F; n = 1; break;
        default:   res = *p;        n = 0; break;
    }
    while(n--) res = (res << 6) | (*(++p) & 0x3F);
    *dst = res;
    return p + 1;
}

s32 codepoint_to_utf8(cstr p, u32 cp) {
    if(cp <= 0x7F) {
        p[0] = (char) cp;
        p[1] = 0;
        return 1;
    } else if(cp <= 0x07FF) {
        p[0] = (char) (((cp >> 6) & 0x1F) | 0xC0);
        p[1] = (char) (((cp >> 0) & 0x3F) | 0x80);
        p[2] = 0;
        return 2;
    } else if(cp <= 0xFFFF) {
        p[0] = (char) (((cp >> 12) & 0x0F) | 0xE0);
        p[1] = (char) (((cp >>  6) & 0x3F) | 0x80);
        p[2] = (char) (((cp >>  0) & 0x3F) | 0x80);
        p[3] = 0;
        return 3;
    } else if(cp <= 0x10FFFF) {
        p[0] = (char) (((cp >> 18) & 0x07) | 0xF0);
        p[1] = (char) (((cp >> 12) & 0x3F) | 0x80);
        p[2] = (char) (((cp >>  6) & 0x3F) | 0x80);
        p[3] = (char) (((cp >>  0) & 0x3F) | 0x80);
        p[4] = 0;
        return 4;
    } else { 
        p[0] = (char) 0xEF;  
        p[1] = (char) 0xBF;
        p[2] = (char) 0xBD;
        p[3] = 0;
        return 0;
    }
}


struct UTF8_Decoderator {
    struct It {
        It() : p(NULL), cp(0), valid(false) {}
        It(cstr _p) : p(_p), cp(0), valid(*p != 0) { if(valid) next(); }

        u32 operator*() const { return cp; }

        It& operator++() {
            if(valid) next();
            return *this;
        }

        It operator++(int) {
            auto it = *this;
            operator++();
            return it;
        }

        bool operator==(It const& b) const {
            if(p != b.p) return false;
            if(valid != b.valid) return false;
            return cp == b.cp;
        }

        bool operator!=(It const& b) const { return !(*this == b); }

    private:
        cstr p;
        u32 cp;
        bool valid;

        void next() {
            cp = 0;
            valid = false;
            if(*p != 0) {
                p = utf8_to_codepoint(p, &cp);
                valid = true;
            }
        }
    };

    UTF8_Decoderator(cstr _p) : p(_p) {}

    It begin() const { return It(p); }
    It end() const { return It(p + strlen(p)); }

private:
    cstr p;

    bool more() const { return *p != 0; }

    u32 next() {
        assert(more());
        u32 cp;
        p = utf8_to_codepoint(p, &cp);
        return cp;
    }
};