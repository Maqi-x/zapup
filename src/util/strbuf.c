#include <util/strbuf.h>

#include <stdlib.h>
#include <string.h>

#define Z_STRBUF_DEFAULT_CAP 16

static usize _z_strbuf_next_cap(usize current_cap, usize required_min_cap) {
    usize new_cap = current_cap == 0 ? Z_STRBUF_DEFAULT_CAP : current_cap * 2;
    if (new_cap < required_min_cap) {
        new_cap = required_min_cap;
    }
    return new_cap;
} 

bool z_strbuf_init(ZStringBuf* sb) {
    return z_strbuf_init_with_cap(sb, Z_STRBUF_DEFAULT_CAP);
}

bool z_strbuf_init_with_cap(ZStringBuf* sb, usize init_cap) {
    sb->data = malloc(init_cap);
    if (sb->data == NULL) return false;
    sb->len = 0;
    sb->cap = init_cap;
    return true;
}

bool z_strbuf_init_from(ZStringBuf* sb, ZStringView sv) {
    sb->data = malloc(sv.len);
    if (sb->data == NULL) return false;

    memcpy(sb->data, sv.data, sv.len);
    sb->len = sv.len;
    sb->cap = sv.len;
    return true;
}

bool z_strbuf_init_from_cstr(ZStringBuf* sb, const char* cstr) {
    return z_strbuf_init_from(sb, z_sv_from_cstr(cstr));
}

void z_strbuf_destroy(ZStringBuf* sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}

bool z_strbuf_copy(const ZStringBuf* src, ZStringBuf* dst) {
    if (src->len == 0) {
        dst->data = NULL;
        dst->len = 0;
        dst->cap = 0;
        return true;
    }
    dst->data = malloc(src->len);
    if (dst->data == NULL) {
        dst->len = 0;
        dst->cap = 0;
        return false;
    }
    memcpy(dst->data, src->data, src->len);
    dst->len = src->len;
    dst->cap = src->len;
    return true;
}

void z_strbuf_move(ZStringBuf* src, ZStringBuf* dst) {
    *dst = *src;
    src->data = NULL;
    src->len = 0;
    src->cap = 0;
}

bool z_strbuf_resize(ZStringBuf* sb, usize new_len) {
    if (new_len > sb->cap) {
        if (!z_strbuf_reserve_exact(sb, new_len)) {
            return false;
        }
    }
    sb->len = new_len;
    return true;
}

bool z_strbuf_reserve(ZStringBuf* sb, usize min_cap) {
    if (min_cap <= sb->cap) {
        return true;
    }

    usize new_cap = _z_strbuf_next_cap(sb->cap, min_cap);
    char* new_data = realloc(sb->data, new_cap);
    if (new_data == NULL) return false;

    sb->data = new_data;
    sb->cap = new_cap;
    return true;
}

bool z_strbuf_reserve_exact(ZStringBuf* sb, usize new_cap) {
    if (new_cap <= sb->cap) {
        return true;
    }
    
    char* new_data = realloc(sb->data, new_cap);
    if (new_data == NULL) return false;

    sb->data = new_data;
    sb->cap = new_cap;
    return true;
}

void z_strbuf_clear(ZStringBuf* sb) {
    sb->len = 0;
}

bool z_strbuf_append(ZStringBuf* sb, ZStringView sv) {
    if (sv.len == 0) return true;
    if (!z_strbuf_reserve(sb, sb->len + sv.len)) {
        return false;
    }

    if (!memcpy(sb->data + sb->len, sv.data, sv.len)) {
        return false;
    }
    sb->len += sv.len;
    return true;
}

bool z_strbuf_append_char(ZStringBuf* sb, char c) {
    if (!z_strbuf_reserve(sb, sb->len + 1)) {
        return false;
    }
    sb->data[sb->len] = c;
    sb->len += 1;
    return true;
}
