#pragma once

#include <util/strbuf.h>
#include <defs/sv.h>

#include <stdbool.h>

typedef enum ZBuildType {
    Z_BUILD_RELEASE,
    Z_BUILD_DEBUG,
} ZBuildType;

typedef struct ZapVersion {
    ZStringView branch;
    ZStringView revspec;
    ZBuildType build;
} ZapVersion;

#define Z_ZAP_VERSION_NULL ((ZapVersion) { .branch = Z_SV_NULL, .revspec = Z_SV_NULL })

static inline bool z_zap_ver_is_null(ZapVersion v) {
    return z_sv_is_null(v.branch) && z_sv_is_null(v.revspec);
}

ZapVersion z_parse_zap_version(ZStringView s);
bool z_format_zap_version(ZapVersion v, ZStringBuf* out);
