#pragma once

#include <util/strbuf.h>
#include <defs/sv.h>

#include <stdbool.h>

typedef enum ZBuildType {
    Z_BUILD_RELEASE,
    Z_BUILD_DEBUG,
} ZBuildType;

typedef struct ZResolvableZapVersion {
    ZStringView branch;
    ZStringView commit;
    ZBuildType build;
} ZResolvableZapVersion;

#define Z_ZAP_VERSION_NULL ((ZResolvableZapVersion) { .branch = Z_SV_NULL, .commit = Z_SV_NULL })

static inline bool z_zap_ver_is_null(ZResolvableZapVersion v) {
    return z_sv_is_null(v.branch) && z_sv_is_null(v.commit);
}

ZResolvableZapVersion z_parse_zap_version(ZStringView s);
bool z_format_zap_version(ZResolvableZapVersion v, ZStringBuf* out);
