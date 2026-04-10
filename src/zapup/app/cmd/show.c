#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>

int zapup_exec_show(ZapupApp* app) {
    ZapVersion ver = app->cfg.toolchain.active_version;
    if (z_zap_ver_is_null(ver)) {
        z_show_error("No selected version");
        return 1;
    }

    ZVersionIndexEntry* entry = z_version_index_find_by_version(&app->index, ver);
    if (entry == NULL) {
        z_show_error("Selected version not in index (invalid config)");
        return 2;
    }
    
    ZStringBuf version_formatted;
    z_strbuf_init(&version_formatted);
    z_format_zap_version(ver, &version_formatted);
    
    z_show_info("Currently installed: " Z_SV_FMT, Z_SV_FARG(z_strbuf_view(&version_formatted)));
    z_show_info("Path: " Z_SV_FMT, Z_SV_FARG(z_strbuf_view(&entry->path)));

    z_strbuf_destroy(&version_formatted);
    return 0;
}
