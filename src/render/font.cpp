// TODO: Make this code robust enough that I can sort of trust it lol, at least somewhat
// TODO: Make it convenient to use fonts at numerous different sizes

#define MAX_GLYPHSETS 256

struct GlyphSet {
    stbtt_bakedchar glyphs[256];
    s32 size;
    GLuint tex;
};

GlyphSet* get_glyphset(struct Font *font, s32 codepoint);

struct Font {
    u8 *ttf_data;
    stbtt_fontinfo info;
    GlyphSet* glyph_sets[MAX_GLYPHSETS];
    f32 size;
    s32 height;

    f32 width(cstr s) {
        f32 xp = 0;
        cstr p = s;
        while(*p) {
            u32 codepoint;
            p = utf8_to_codepoint(p, &codepoint);
            auto set = get_glyphset(this, codepoint);
            auto g = &set->glyphs[codepoint & 0xFF];
            xp += g->xadvance;
        }
        return xp;
    }

    f32 width(rstr s) {
        return width(cast(cstr, s));
    }
};

GlyphSet* load_glyphset(Font *font, s32 index) {
    auto result = (GlyphSet*)calloc(1, sizeof(GlyphSet));

    f32 s = stbtt_ScaleForMappingEmToPixels(&font->info, 1) / stbtt_ScaleForPixelHeight(&font->info, 1);
    s32 size = 128;
    u8 *raw_image = 0;
    s32 f;
    do {
        raw_image = (u8*)malloc(square(size) * sizeof(u32));

        f = stbtt_BakeFontBitmap(font->ttf_data, 0, font->size * s, raw_image, size, size, index * 256, 256, result->glyphs);

        if(f <= 0) {
            size *= 2;
            free(raw_image);
        }
    } while(f <= 0);
    result->size = size;

    u32 *image = (u32*) raw_image;
    for(s32 i = size * size - 1; i >= 0; i--) {
        u8 p = raw_image[i] & 0xFF;
        image[i] = (p << 24) | (p << 16) | (p << 8) | p;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &result->tex);
    glTextureStorage2D(result->tex, 1, GL_RGBA8, size, size);
    glTextureSubImage2D(result->tex, 0, 0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, raw_image);
    glTextureParameteri(result->tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(result->tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(result->tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(result->tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

    free(raw_image);

    s32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &line_gap);
    f32 scale = stbtt_ScaleForMappingEmToPixels(&font->info, font->size);
    s32 scaled_ascent = ascent * scale + 0.5;
    for(s32 i = 0; i < 256; i++) {
        result->glyphs[i].yoff += scaled_ascent;
        result->glyphs[i].xadvance = floor(result->glyphs[i].xadvance);
    }

    return result;
}

void free_glyphset(GlyphSet *set) {
    free(set);
}

GlyphSet* get_glyphset(Font *font, s32 codepoint) {
    s32 index = (codepoint >> 8) % MAX_GLYPHSETS;
    if(!font->glyph_sets[index]) {
        font->glyph_sets[index] = load_glyphset(font, index);
    }
    return font->glyph_sets[index];
}

Font* load_font(cstr file, f32 size) {
    auto file_data = read_entire_file(file);
    if(!file_data) return NULL;
    
    auto result = xnew(Font);
    result->size = size;
    result->ttf_data = (u8*)file_data;

    if(!stbtt_InitFont(&result->info, result->ttf_data, 0)) {
        xfree(result);
        xfree(strhdr(file_data));
        return 0;
    }

    s32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&result->info, &ascent, &descent, &line_gap);
    f32 scale = stbtt_ScaleForMappingEmToPixels(&result->info, size);
    result->height = (ascent - descent + line_gap) * scale + 0.5;

    stbtt_bakedchar *g = get_glyphset(result, '\n')->glyphs;
    g[(s32)'\t'].x1 = g[(s32)'\t'].x0;
    g[(s32)'\n'].x1 = g[(s32)'\n'].x0;

    return result;
}

Font* load_font(rstr file, f32 size) {
    return load_font(cast(cstr, file), size);
}

void free_font(Font *font) {
    xfree(strhdr(font->ttf_data));
    for(s32 i = 0; i < MAX_GLYPHSETS; i++) {
        auto set = font->glyph_sets[i];
        if(set) free_glyphset(set);
    }
    xfree(font);
}

f32 draw_codepoint(Batch_Renderer *r, Font *font, u32 codepoint, f32 x, f32 y, vec4 color) {
    f32 xp = x;
    auto *set = get_glyphset(font, codepoint);
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(set->glyphs, set->size, set->size, codepoint & 0xFF, &xp, &y, &q, 1);
    vec2 uvs[] = {
        vec2(q.s0, q.t0),
        vec2(q.s1, q.t0),
        vec2(q.s1, q.t1),
        vec2(q.s0, q.t1)
    };
    r->push_quad(
        q.x0, q.y0,
        (f32)(q.x1 - q.x0), (f32)(q.y1 - q.y0),
        color, uvs, set->tex
    );
    return xp;
}

f32 draw_text(Batch_Renderer *r, Font *font, cstr text, f32 x, f32 y, vec4 color) {
    f32 xp = x;
    auto d = UTF8_Decoderator(text);
    for(auto cp : d) {
       xp = draw_codepoint(r, font, cp, xp, y, color);
    }
    return xp;
}

f32 draw_text(Batch_Renderer *r, Font *font, rstr text, f32 x, f32 y, vec4 color) {
    return draw_text(r, font, cast(cstr, text), x, y, color);
}