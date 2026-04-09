#include <zapup/zap/version.h>

ZapVersion z_parse_zap_version(ZStringView s) {
    if (s.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }

    ZapVersion v = {
        .branch = Z_SV_NULL,
        .ref_kind = Z_REF_REVSPEC,
        .revspec = s,
        .build = Z_BUILD_RELEASE,
    };

    if (z_sv_ends_with(s, Z_SV(":debug"))) {
        v.build = Z_BUILD_DEBUG;
        v.revspec.len -= 6;
    } else if (z_sv_ends_with(s, Z_SV(":release"))) {
        v.build = Z_BUILD_RELEASE;
        v.revspec.len -= 8;
    }

    if (v.revspec.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }

    for (usize i = 0; i < v.revspec.len; ++i) {
        if (v.revspec.data[i] == '@') {
            v.branch = z_sv_slice(v.revspec, 0, i);
            v.revspec = z_sv_slice(v.revspec, i + 1, v.revspec.len);
            break;
        }
    }

    if (v.revspec.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }

    if (z_sv_eql(v.revspec, Z_SV("latest")) || z_sv_eql(v.revspec, Z_SV("HEAD"))) {
        v.ref_kind = Z_REF_LATEST;
        v.revspec = Z_SV_NULL;
    } else {
        v.ref_kind = Z_REF_REVSPEC;
    }

    return v;
}

bool z_format_zap_version(ZapVersion v, ZStringBuf* out) {
    if (z_zap_ver_is_null(v)) {
        return false;
    }

    if (!z_sv_is_null(v.branch)) {
        if (!z_strbuf_append(out, v.branch)) return false;
        if (!z_strbuf_append_char(out, '@')) return false;
    }

    if (v.ref_kind == Z_REF_REVSPEC) {
        if (!z_strbuf_append(out, v.revspec)) return false;
    } else if (v.ref_kind == Z_REF_LATEST) {
        if (!z_strbuf_append_cstr(out, "latest")) return false;
    }

    if (v.build == Z_BUILD_DEBUG) {
        if (!z_strbuf_append_cstr(out, ":debug")) {
            return false;
        }
    }
    return true;
}
