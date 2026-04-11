#pragma once

#include <util/pathview.h>
#include <zapup/zap/version.h>

typedef enum ZapBuildErrorCode {
    Z_ZAP_BUILD_SUCCESS,
    Z_ZAP_BUILD_FS_ERROR,
    Z_ZAP_BUILD_CONFIGURE_ERR,
    Z_ZAP_BUILD_COMPILATION_ERR,
    Z_ZAP_BUILD_TESTS_ERR,
} ZapBuildErrorCode;

typedef struct ZapBuildResult {
    ZapBuildErrorCode code;
} ZapBuildResult;

typedef struct ZapBuildOptions {
    ZPathView zap_root;
    ZapVersion ver;
    bool parallel;
    int max_jobs;
    bool run_tests;
    ZStringView cc;
    ZStringView cxx;
} ZapBuildOptions;

ZapBuildResult z_cmake_build_zap(const ZapBuildOptions* opts);
