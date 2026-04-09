#pragma once

#include <util/strbuf.h>
#include <defs/sv.h>

#include <stdbool.h>

typedef enum ZBuildType {
    Z_BUILD_RELEASE,
    Z_BUILD_DEBUG,
} ZBuildType;

typedef enum ZRefKind {
    Z_REF_NONE,
    Z_REF_LATEST,
    Z_REF_STABLE,
    Z_REF_REVSPEC,
} ZRefKind;

typedef struct ZapVersion {
    ZStringView branch;
    ZRefKind ref_kind;
    ZStringView revspec;
    ZBuildType build;
} ZapVersion;

#define Z_ZAP_VERSION_NULL ((ZapVersion) { .branch = Z_SV_NULL, .ref_kind = Z_REF_NONE, .revspec = Z_SV_NULL, .build = Z_BUILD_RELEASE })

static inline bool z_zap_ver_is_null(ZapVersion v) {
    return z_sv_is_null(v.branch) && v.ref_kind == Z_REF_NONE;
}

static inline bool z_zap_ver_is_dynamic(ZapVersion v) {
    return v.ref_kind == Z_REF_LATEST || v.ref_kind == Z_REF_STABLE;
}

ZapVersion z_parse_zap_version(ZStringView s);
bool z_format_zap_version(ZapVersion v, ZStringBuf* out);
