#include <util/pathview.h>
#include <ctype.h>

ZPathView z_pathview_dirname(ZPathView pv) {
    if (pv.len == 0) return pv;

    isize last_sep = -1;
    for (isize i = (isize)pv.len - 1; i >= 0; i--) {
        if (Z_IS_PATH_SEP(pv.data[i])) {
            last_sep = i;
            break;
        }
    }

    if (last_sep == -1) {
#if Z_PLATFORM_IS_WINDOWS
        if (pv.len >= 2 && pv.data[1] == ':') {
            return z_sv_slice(pv, 0, 2);
        }
#endif
        return Z_PV_NULL;
    }

    if (last_sep == 0) {
        return z_sv_slice(pv, 0, 1);
    }

#if Z_PLATFORM_IS_WINDOWS
    if (last_sep == 2 && pv.len >= 2 && pv.data[1] == ':') {
        return z_sv_slice(pv, 0, 3);
    }
#endif

    return z_sv_slice(pv, 0, (usize)last_sep);
}

ZPathView z_pathview_basename(ZPathView pv) {
    if (pv.len == 0) return pv;

    isize last_sep = -1;
    for (isize i = (isize)pv.len - 1; i >= 0; i--) {
        if (Z_IS_PATH_SEP(pv.data[i])) {
            last_sep = i;
            break;
        }
    }

    if (last_sep == -1) {
#if Z_PLATFORM_IS_WINDOWS
        if (pv.len >= 2 && pv.data[1] == ':') {
            return z_sv_slice(pv, 2, pv.len);
        }
#endif
        return pv;
    }

    return z_sv_slice(pv, (usize)last_sep + 1, pv.len);
}

ZPathView z_pathview_ext(ZPathView pv) {
    ZPathView base = z_pathview_basename(pv);
    isize last_dot = -1;
    for (isize i = (isize)base.len - 1; i >= 0; i--) {
        if (base.data[i] == '.') {
            last_dot = i;
            break;
        }
    }

    if (last_dot <= 0) {
        return Z_PV_NULL;
    }

    return z_sv_slice(base, (usize)last_dot, base.len);
}

ZPathView z_pathview_stem(ZPathView pv) {
    ZPathView base = z_pathview_basename(pv);
    ZPathView ext = z_pathview_ext(base);
    return z_sv_slice(base, 0, base.len - ext.len);
}

bool z_pathview_is_absolute(ZPathView pv) {
#if Z_PLATFORM_IS_WINDOWS
    if (pv.len >= 2 && isalpha(pv.data[0]) && pv.data[1] == ':') {
        return pv.len >= 3 && Z_IS_PATH_SEP(pv.data[2]);
    }
    return pv.len >= 2 && Z_IS_PATH_SEP(pv.data[0]) && Z_IS_PATH_SEP(pv.data[1]);
#else
    return pv.len > 0 && pv.data[0] == '/';
#endif
}

bool z_pathview_is_root(ZPathView pv) {
#if Z_PLATFORM_IS_WINDOWS
    if (pv.len == 3 && isalpha(pv.data[0]) && pv.data[1] == ':' && Z_IS_PATH_SEP(pv.data[2])) {
        return true;
    }
    if (pv.len == 1 && Z_IS_PATH_SEP(pv.data[0])) {
        return true;
    }
    return false;
#else
    return pv.len == 1 && pv.data[0] == '/';
#endif
}

bool z_pathview_has_sep_at_end(ZPathView pv) {
    return pv.len > 0 && Z_IS_PATH_SEP(pv.data[pv.len - 1]);
}
