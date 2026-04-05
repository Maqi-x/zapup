#include <util/pathbuf.h>

bool z_pathbuf_append(ZPathBuf* pb, ZPathView pv) {
    return z_strbuf_append(pb, pv);
}

bool z_pathbuf_join(ZPathBuf* pb, ZPathView pv) {
    if (pv.len == 0) return true;

    if (pb->len > 0) {
        bool buf_has_sep = Z_IS_PATH_SEP(pb->data[pb->len - 1]);
        bool view_has_sep = Z_IS_PATH_SEP(pv.data[0]);

        if (!buf_has_sep && !view_has_sep) {
            if (!z_strbuf_append_char(pb, Z_PATH_SEP)) {
                return false;
            }
        } else if (buf_has_sep && view_has_sep) {
            pv = z_sv_slice(pv, 1, pv.len);
        }
    }

    return z_strbuf_append(pb, pv);
}

bool z_pathbuf_join_array(ZPathBuf* pb, ZPathViewArray parts) {
    for (usize i = 0; i < parts.count; i++) {
        if (!z_pathbuf_join(pb, parts.data[i])) {
            return false;
        }
    }
    return true;
}

bool z_pathbuf_init_from_parts(ZPathBuf* pb, ZPathViewArray parts) {
    if (!z_pathbuf_init(pb)) {
        return false;
    }
    if (!z_pathbuf_join_array(pb, parts)) {
        z_pathbuf_destroy(pb);
        return false;
    }
    return true;
}

bool z_pathbuf_pop(ZPathBuf* pb) {
    ZPathView view = z_pathbuf_as_view(pb);
    ZPathView dir = z_pathview_dirname(view);

    if (dir.len == 0 && pb->len > 0) {
        if (z_pathview_is_root(view)) {
            return true;
        }
        z_strbuf_clear(pb);
        return true;
    }

    return z_strbuf_resize(pb, dir.len);
}

bool z_pathbuf_set_ext(ZPathBuf* pb, ZPathView ext) {
    ZPathView current_view = z_pathbuf_as_view(pb);
    ZPathView current_ext = z_pathview_ext(current_view);

    if (current_ext.len > 0) {
        if (!z_strbuf_resize(pb, pb->len - current_ext.len)) {
            return false;
        }
    }

    if (ext.len > 0) {
        if (ext.data[0] != '.') {
            if (!z_strbuf_append_char(pb, '.')) {
                return false;
            }
        }
        return z_strbuf_append(pb, ext);
    }

    return true;
}
