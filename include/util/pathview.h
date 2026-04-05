#pragma once

#include <defs/sv.h>
#include <defs/sv-arr.h>
#include <defs/platform.h>

typedef ZStringView ZPathView;
typedef ZStringViewArray ZPathViewArray;

#define Z_PATH_VIEWS(...)                                                     \
    (ZPathViewArray) {                                                        \
        .data = (ZPathView[]) { __VA_ARGS__ },                                \
        .count = sizeof((ZPathView[]) { __VA_ARGS__ }) / sizeof(ZPathView)    \
    }

#define Z_PV_NULL Z_SV_NULL
#define Z_PV(LIT) Z_SV(LIT)

#if Z_PLATFORM_IS_WINDOWS
#  define Z_PATH_SEP '\\'
#  define Z_PATH_SEP_STR "\\"
#  define Z_IS_PATH_SEP(c) ((c) == '/' || (c) == '\\')
#else
#  define Z_PATH_SEP '/'
#  define Z_PATH_SEP_STR "/"
#  define Z_IS_PATH_SEP(c) ((c) == '/')
#endif

static inline ZPathView z_pathview_from_sv(ZStringView sv) {
    return sv;
}

static inline ZPathView z_pathview_from_cstr(const char* cstr) {
    return z_sv_from_cstr(cstr);
}

ZPathView z_pathview_dirname(ZPathView pv);
ZPathView z_pathview_basename(ZPathView pv);
ZPathView z_pathview_stem(ZPathView pv);
ZPathView z_pathview_ext(ZPathView pv);

bool z_pathview_is_absolute(ZPathView pv);
bool z_pathview_is_root(ZPathView pv);
bool z_pathview_has_sep_at_end(ZPathView pv);

static inline bool z_pathview_is_relative(ZPathView pv) {
    return !z_pathview_is_absolute(pv);
}
