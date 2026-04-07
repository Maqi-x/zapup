#include <zapup/zap/version.h>

ZResolvableZapVersion z_parse_zap_version(ZStringView s) {
    if (s.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }

    ZResolvableZapVersion v = {
        .branch = Z_SV_NULL,
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
    if (z_sv_eql(v.revspec, Z_SV("latest"))) {
        v.revspec = Z_SV("HEAD");
    }
    return v;
}

bool z_format_zap_version(ZResolvableZapVersion v, ZStringBuf* out) {
    if (z_zap_ver_is_null(v)) {
        return false;
    }

    if (!z_sv_is_null(v.branch)) {
        if (!z_strbuf_append(out, v.branch)) return false;
        if (!z_strbuf_append_char(out, '@')) return false;
    }

    if (!z_strbuf_append(out, v.revspec)) return false;

    switch (v.build) {
    case Z_BUILD_DEBUG:
        return z_strbuf_append_cstr(out, ":debug");
    case Z_BUILD_RELEASE:
        return z_strbuf_append_cstr(out, ":release");
    }
}
