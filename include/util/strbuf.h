#pragma once

#include <util/fmt-arg.h>
#include <defs/sv.h>

#include <stdbool.h>
#include <stdarg.h>

typedef struct ZStringBuf {
    char* data;
    usize len;
    usize cap;
} ZStringBuf;

bool z_strbuf_init(ZStringBuf* sb);
bool z_strbuf_init_with_cap(ZStringBuf* sb, usize init_cap);
bool z_strbuf_init_from(ZStringBuf* sb, ZStringView sv);
bool z_strbuf_init_from_cstr(ZStringBuf* sb, const char* cstr);
void z_strbuf_destroy(ZStringBuf* sb);

bool z_strbuf_copy(const ZStringBuf* src, ZStringBuf* dst);
void z_strbuf_move(ZStringBuf* src, ZStringBuf* dst);

static inline ZStringView z_strbuf_view(const ZStringBuf* sb) {
    return z_sv_from_data_and_len(sb->data, sb->len);
}

bool z_strbuf_resize(ZStringBuf* sb, usize new_len);
bool z_strbuf_reserve(ZStringBuf* sb, usize min_cap);
bool z_strbuf_reserve_exact(ZStringBuf* sb, usize new_cap);

void z_strbuf_clear(ZStringBuf* sb);

bool z_strbuf_append(ZStringBuf* sb, ZStringView sv);
static inline bool z_strbuf_append_cstr(ZStringBuf* sb, const char* cstr) {
    return z_strbuf_append(sb, z_sv_from_cstr(cstr));
}
static inline bool z_strbuf_append_buf(ZStringBuf* sb, const ZStringBuf* other) {
    return z_strbuf_append(sb, z_strbuf_view(other));
}

bool z_strbuf_append_char(ZStringBuf* sb, char c);

static inline bool z_strbuf_eql_to(const ZStringBuf* lhs, ZStringView rhs) {
    return z_sv_eql(z_strbuf_view(lhs), rhs);
}
static inline bool z_strbuf_eql_to_cstr(const ZStringBuf* lhs, const char* rhs) {
    return z_strbuf_eql_to(lhs, z_sv_from_cstr(rhs));
}
static inline bool z_strbuf_eql_to_buf(const ZStringBuf* lhs, const ZStringBuf* rhs) {
    return z_strbuf_eql_to(lhs, z_strbuf_view(rhs));
}

static inline bool z_strbuf_starts_with(const ZStringBuf* sb, ZStringView prefix) {
    return z_sv_starts_with(z_strbuf_view(sb), prefix);
}
static inline bool z_strbuf_ends_with(const ZStringBuf* sb, ZStringView suffix) {
    return z_sv_ends_with(z_strbuf_view(sb), suffix);
}

bool z_strbuf_vappendf(ZStringBuf* sb, const char* fmt, va_list args);
bool z_strbuf_appendf(ZStringBuf* sb, const char* fmt, ...) Z_PRINTF_FUNCTION(2, 3);
