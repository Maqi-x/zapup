#pragma once

#include "util/strbuf.h"
#include "util/pathview.h"

typedef ZStringBuf ZPathBuf;

static inline bool z_pathbuf_init(ZPathBuf* pb) {
    return z_strbuf_init(pb);
}

static inline bool z_pathbuf_init_from(ZPathBuf* pb, ZPathView pv) {
    return z_strbuf_init_from(pb, pv);
}

static inline void z_pathbuf_destroy(ZPathBuf* pb) {
    z_strbuf_destroy(pb);
}

static inline void z_pathbuf_clear(ZPathBuf* pb) {
    z_strbuf_clear(pb);
}

static inline ZPathView z_pathbuf_as_view(const ZPathBuf* pb) {
    return z_strbuf_view(pb);
}

bool z_pathbuf_join(ZPathBuf* pb, ZPathView pv);
bool z_pathbuf_join_array(ZPathBuf* pb, ZPathViewArray parts);
bool z_pathbuf_append(ZPathBuf* pb, ZPathView pv);
bool z_pathbuf_pop(ZPathBuf* pb);
bool z_pathbuf_set_ext(ZPathBuf* pb, ZPathView ext);

void z_pathbuf_sanitize(ZPathBuf* pb);

bool z_pathbuf_init_from_parts(ZPathBuf* pb, ZPathViewArray parts);
