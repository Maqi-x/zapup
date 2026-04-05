#pragma once

#include <defs/sv.h>

typedef struct ZResolvableZapVersion {
    ZStringView branch;
    ZStringView commit;
} ZResolvableZapVersion;

#define Z_ZAP_VERSION_NULL ((ZResolvableZapVersion) { .branch = Z_SV_NULL, .commit = Z_SV_NULL })

ZResolvableZapVersion z_parse_zap_version(ZStringView s);
