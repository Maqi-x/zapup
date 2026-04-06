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

    for (usize i = 0; i < v.commit.len; ++i) {
        if (v.commit.data[i] == '@') {
            v.branch = z_sv_slice(v.commit, 0, i);
            v.commit = z_sv_slice(v.commit, i + 1, v.commit.len);
            break;
        }
    }

    return v;
}
