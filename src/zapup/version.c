#include <zapup/version.h>

ZResolvableZapVersion z_parse_zap_version(ZStringView s) {
    if (s.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }

    ZResolvableZapVersion v = {
        .branch = Z_SV_NULL,
        .commit = s,
        .build = Z_BUILD_RELEASE,
    };

    if (z_sv_ends_with(s, Z_SV(":debug"))) {
        v.build = Z_BUILD_DEBUG;
        v.commit.len -= 6;
    } else if (z_sv_ends_with(s, Z_SV(":release"))) {
        v.build = Z_BUILD_RELEASE;
        v.commit.len -= 8;
    }

    if (v.commit.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }

    for (usize i = 0; i < v.commit.len; ++i) {
        if (v.commit.data[i] == '@') {
            v.branch = z_sv_slice(v.commit, 0, i);
            v.commit = z_sv_slice(v.commit, i + 1, v.commit.len);
            break;
        }
    }

    if (v.commit.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }
    if (z_sv_eql(v.commit, Z_SV("latest"))) {
        v.commit = Z_SV("HEAD");
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

    if (!z_strbuf_append(out, v.commit)) return false;

    switch (v.build) {
    case Z_BUILD_DEBUG:
        return z_strbuf_append_cstr(out, ":debug");
    case Z_BUILD_RELEASE:
        return z_strbuf_append_cstr(out, ":release");
    }
}
