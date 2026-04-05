#pragma once

#include <defs/int-types.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ZStringView {
    const char* data;
    usize len;
} ZStringView;

#define Z_SV_NULL ((ZStringView) { .data = NULL, .len = 0 })
#define Z_SV(STRING_LITERAL) ((ZStringView) { .data = STRING_LITERAL, .len = sizeof(STRING_LITERAL) - 1 })

static inline ulong z_sv_print(ZStringView sv, FILE* out) {
    return fwrite(sv.data, 1, sv.len, out);
}

static inline ZStringView z_sv_from_cstr(const char* cstr) {
    return (ZStringView) { .data = cstr, .len = strlen(cstr) };
}

static inline ZStringView z_sv_from_data_and_len(const char* data, usize len) {
    return (ZStringView) { .data = data, .len = len };
}

static inline bool z_sv_is_null(ZStringView sv) {
    return sv.data == NULL;
}

static inline bool z_sv_eql(ZStringView lhs, ZStringView rhs) {
    if (lhs.len != rhs.len) return false;
    if (lhs.data == rhs.data) return true;
    return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

static inline bool z_sv_starts_with(ZStringView sv, ZStringView prefix) {
    if (prefix.len > sv.len) return false;
    return memcmp(sv.data, prefix.data, prefix.len) == 0;
}

static inline bool z_sv_ends_with(ZStringView sv, ZStringView suffix) {
    if (suffix.len > sv.len) return false;
    if (suffix.len == sv.len) return memcmp(sv.data, suffix.data, sv.len);

    return memcmp(sv.data + (sv.len - suffix.len), suffix.data, suffix.len) == 0;
}

static inline ZStringView z_sv_trim_prefix(ZStringView sv, ZStringView prefix) {
    if (!z_sv_starts_with(sv, prefix)) return sv;
    return (ZStringView) { .data = sv.data + prefix.len, .len = sv.len - prefix.len };
}

static inline ZStringView z_sv_trim_suffix(ZStringView sv, ZStringView suffix) {
    if (!z_sv_ends_with(sv, suffix)) return sv;
    return (ZStringView) { .data = sv.data, .len = suffix.len };
}

static inline ZStringView z_sv_trim_prefix_or_null(ZStringView sv, ZStringView prefix) {
    if (!z_sv_starts_with(sv, prefix)) return Z_SV_NULL;
    return (ZStringView) { .data = sv.data + prefix.len, .len = sv.len - prefix.len };
}

static inline ZStringView z_sv_trim_suffix_or_null(ZStringView sv, ZStringView suffix) {
    if (!z_sv_ends_with(sv, suffix)) return Z_SV_NULL;
    return (ZStringView) { .data = sv.data, .len = suffix.len };
}

static inline ZStringView z_sv_slice(ZStringView sv, usize start, usize end) {
    if (sv.data == NULL || start > end || end > sv.len) {
        return Z_SV_NULL;
    }

    return (ZStringView) { .data = sv.data + start, .len = end - start };
}

static inline ZStringView z_sv_window(ZStringView sv, usize start, usize len) {
    if (start >= sv.len) return (ZStringView) { 0 };
    if (start + len > sv.len) len = sv.len - start;

    return (ZStringView) { .data = sv.data + start, .len = len };
}

static inline char* z_sv_to_cstr_alloc(ZStringView sv) {
    if (z_sv_is_null(sv)) return NULL;
    char* cstr = malloc(sv.len + 1);
    if (cstr == NULL) return NULL;
    memcpy(cstr, sv.data, sv.len);
    cstr[sv.len] = '\0';
    return cstr;
}
