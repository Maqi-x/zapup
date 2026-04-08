#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>

int zapup_exec_list(ZapupApp* app) {
    if (app->index.len == 0) {
        z_show_warn("No installed zap versions found");
        return 0;
    }
    for (usize i = 0; i < app->index.len; ++i) {
        ZVersionIndexEntry* entry = &app->index.entries[i];
        ZResolvableZapVersion ver = z_version_index_entry_version(entry);
        
        ZStringBuf version_formatted;
        z_strbuf_init(&version_formatted);
        z_format_zap_version(ver, &version_formatted);

        z_show_info(
            Z_SV_FMT ": " Z_SV_FMT,
            Z_SV_FARG(z_strbuf_view(&version_formatted)),
            Z_SV_FARG(entry->path)
        );
    }
    return 0;
}
