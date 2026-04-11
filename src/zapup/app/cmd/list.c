#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>

int zapup_exec_list(ZapupApp* app) {
    if (app->index.len == 0) {
        z_show_warn("No installed zap versions found");
        return 0;
    }

    ZStringBuf* formatted = malloc(app->index.len * sizeof(ZStringBuf));
    usize max_len = 0;
    for (usize i = 0; i < app->index.len; ++i) {
        ZVersionIndexEntry* entry = &app->index.entries[i];
        ZapVersion ver = z_version_index_entry_version(entry);

        ZStringBuf version_formatted;
        z_strbuf_init(&version_formatted);
        z_format_zap_version(ver, &version_formatted);

        if (version_formatted.len > max_len) {
            max_len = version_formatted.len;
        }
        formatted[i] = version_formatted;
    }

    for (usize i = 0; i < app->index.len; ++i) {
        ZVersionIndexEntry* entry = &app->index.entries[i];
        ZStringBuf version_formatted = formatted[i];

        z_show_info(
            Z_SV_FMT ": %*s" Z_SV_FMT,
            Z_SV_FARG(z_strbuf_view(&version_formatted)),
            (int)(max_len - version_formatted.len),
            "",
            Z_SV_FARG(entry->path)
        );
        z_strbuf_destroy(&formatted[i]);
    }

    free(formatted);
    return 0;
}
