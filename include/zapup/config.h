#pragma once

#include <zapup/zap/version.h>
#include <util/pathview.h>
#include <util/strbuf.h>
#include <defs/sv.h>

typedef struct ZToolchainConfig {
    ZapVersion active_version;
} ZToolchainConfig;

typedef struct ZBuildConfig {
    ZStringView cc;
    ZStringView cxx;
} ZBuildConfig;

typedef struct ZConfig {
    ZToolchainConfig toolchain;
    ZBuildConfig build;
    void* _ctx;
} ZConfig;

void z_config_init(ZConfig* cfg);
void z_config_free(ZConfig* cfg);

void z_config_from_json(ZConfig* cfg, ZStringView json);
void z_config_merge_from_json(ZConfig* cfg, ZStringView json);
bool z_config_to_json(const ZConfig* cfg, ZStringBuf* out);

bool z_config_load(ZConfig* cfg, ZPathView path);
bool z_config_merge_load(ZConfig* cfg, ZPathView path);
bool z_config_save(const ZConfig* cfg, ZPathView path);
