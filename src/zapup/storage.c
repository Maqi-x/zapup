#include <zapup/storage.h>
#include <zapup/info.h>

#include <util/fs.h>
#include <util/pathbuf.h>
#include <defs/platform.h>

#include <stdlib.h>

#if Z_PLATFORM_IS_WINDOWS

static bool z_paths_init_and_join(ZPathBuf* out, const char* base, ZPathView rel) {
    if (!base || base[0] == '\0') return false;
    if (!z_pathbuf_init_from(out, z_pathview_from_cstr(base))) return false;
    if (!z_pathbuf_join(out, rel)) {
        z_pathbuf_destroy(out);
        return false;
    }
    return true;
}

bool z_paths_find_data_dir(ZPathBuf* out) {
    // %APPDATA%\zapup
    return z_paths_init_and_join(out, getenv("APPDATA"), Z_PV(APP_NAME "\\config"));
}
bool z_paths_find_cache_dir(ZPathBuf* out) {
    // %LOCALAPPDATA%\zapup\cache
    return z_paths_init_and_join(out, getenv("LOCALAPPDATA"), Z_PV(APP_NAME "\\cache"));
}
bool z_paths_find_config_dir(ZPathBuf* out) {
    // %APPDATA%\zapup
    return z_paths_init_and_join(out, getenv("APPDATA"), Z_PV(APP_NAME "\\data"));
}

#else

static bool z_paths_find_xdg_dir(ZPathBuf* out, const char* xdg_env, ZPathView fallback_rel) {
    const char* xdg = getenv(xdg_env);
    if (xdg && xdg[0] != '\0') {
        if (!z_pathbuf_init_from(out, z_pathview_from_cstr(xdg))) return false;
        return z_pathbuf_join(out, Z_PV(APP_NAME));
    }

    const char* home = getenv("HOME");
    if (!home || home[0] == '\0') return false;
    if (!z_pathbuf_init_from(out, z_pathview_from_cstr(home))) return false;
    if (!z_pathbuf_join(out, fallback_rel)) return false;
    return z_pathbuf_join(out, Z_PV(APP_NAME));
}

bool z_paths_find_data_dir(ZPathBuf* out) {
    return z_paths_find_xdg_dir(out, "XDG_DATA_HOME", Z_PV(".local/share"));
}
bool z_paths_find_cache_dir(ZPathBuf* out) {
    return z_paths_find_xdg_dir(out, "XDG_CACHE_HOME", Z_PV(".cache"));
}
bool z_paths_find_config_dir(ZPathBuf* out) {
    return z_paths_find_xdg_dir(out, "XDG_CONFIG_HOME", Z_PV(".config"));
}

#endif

bool z_paths_find_versions_dir(ZPathBuf* out) {
    if (!z_paths_find_data_dir(out)) return false;
    return z_pathbuf_join(out, Z_PV("versions"));
}

bool z_paths_find_index_file(ZPathBuf* out) {
    if (!z_paths_find_versions_dir(out)) return false;
    return z_pathbuf_join(out, Z_PV("index.json"));
}

bool z_paths_config_load(ZPathsConfig* cfg) {
    if (!z_paths_find_config_dir(&cfg->config))  return false;
    if (!z_paths_find_data_dir(&cfg->data))      return false;
    if (!z_paths_find_cache_dir(&cfg->cache))    return false;
    if (!z_paths_find_versions_dir(&cfg->cache)) return false;
    if (!z_paths_find_index_file(&cfg->cache))   return false;
    return true;
}

bool z_paths_ensure_exists(ZPathsConfig* cfg) {
    if (!z_mkdir_all(z_pathbuf_as_view(&cfg->cache)))    return false;
    if (!z_mkdir_all(z_pathbuf_as_view(&cfg->data)))     return false;
    if (!z_mkdir_all(z_pathbuf_as_view(&cfg->config)))   return false;
    if (!z_mkdir_all(z_pathbuf_as_view(&cfg->versions))) return false;
    if (!z_mkfile(z_pathbuf_as_view(&cfg->indexfile)))   return false;
    return true;
}

void z_paths_config_destroy(ZPathsConfig* cfg) {
    z_pathbuf_destroy(&cfg->data);
    z_pathbuf_destroy(&cfg->cache);
    z_pathbuf_destroy(&cfg->config);
    z_pathbuf_destroy(&cfg->versions);
    z_pathbuf_destroy(&cfg->indexfile);
}
