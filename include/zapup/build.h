#pragma once

#include <util/pathview.h>
#include <zapup/version.h>

typedef enum ZCMakeZapBuildErrorCode {
    Z_ZAP_BUILD_SUCCESS,
    Z_ZAP_BUILD_FS_ERROR,
    Z_ZAP_BUILD_CONFIGURE_ERR,
    Z_ZAP_BUILD_COMPILATION_ERR,
} ZCMakeZapBuildErrorCode;

typedef struct ZCMakeZapBuildResult {
    ZCMakeZapBuildErrorCode code;
} ZCMakeZapBuildResult;

typedef struct ZCMakeZapBuildOptions {
    ZPathView zap_root;
    ZResolvableZapVersion ver;
} ZCMakeZapBuildOptions;

ZCMakeZapBuildResult z_cmake_build_zap(const ZCMakeZapBuildOptions* opts);
