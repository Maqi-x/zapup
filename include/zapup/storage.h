#pragma once

#include <util/pathview.h>
#include <util/pathbuf.h>

typedef struct ZPathsConfig {
    ZPathBuf data;
    ZPathBuf cache;
    ZPathBuf config;
    ZPathBuf versions;
    ZPathBuf shims;

    ZPathBuf cfgfile;
    ZPathBuf lspcfg;
    ZPathBuf indexfile;
    ZPathBuf indexlock;
} ZPathsConfig;

bool z_paths_config_load(ZPathsConfig* cfg);
bool z_paths_ensure_exists(ZPathsConfig* cfg);
void z_paths_config_destroy(ZPathsConfig* cfg);
