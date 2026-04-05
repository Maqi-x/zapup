#pragma once

#include <zapup/storage.h>

#include <util/fs.h>

bool z_paths_find_data_dir(ZPathBuf* out) {}
bool z_paths_find_cache_dir(ZPathBuf* out) {}
bool z_paths_find_config_dir(ZPathBuf* out) {}

bool z_paths_config_load(ZPathsConfig* cfg) {
    if (!z_paths_find_config_dir(&cfg->config)) return false;
    if (!z_paths_find_data_dir(&cfg->data))     return false;
    if (!z_paths_find_cache_dir(&cfg->cache))   return false;
    return true;
}

bool z_paths_ensure_exists(ZPathsConfig* cfg) {
    if (!z_mkdir_all(z_pathbuf_as_view(&cfg->cache)))  return false;
    if (!z_mkdir_all(z_pathbuf_as_view(&cfg->data)))   return false;
    if (!z_mkdir_all(z_pathbuf_as_view(&cfg->config))) return false;
    return true;
}
