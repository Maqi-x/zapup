#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>
#include <zapup/zap/sync.h>

int zapup_sync_single_entry(ZapupApp* app, ZVersionIndexEntry* entry) {
    (void) app, (void) entry;
    // TODO
    return 0;
}

int zapup_exec_sync(ZapupApp* app) {
    int res = zapup_ensure_index_lock(app);
    if (res != 0) return res;

    const ZCliSyncArgs* args = &app->args.cmd_args.sync;
    if (z_zap_ver_is_null(args->version)) {
        for (usize i = 0; i < app->index.len; ++i) {
            int result = zapup_sync_single_entry(app, &app->index.entries[i]);
            if (result != 0) {
                z_lockfile_unlock(&app->indexlock);
                return result;
            }
        }
    } else {
        ZVersionIndexEntry* entry = z_version_index_find_by_version(&app->index, args->version);
        if (!entry) {
            ZStringBuf version_formatted;
            z_strbuf_init(&version_formatted);
            z_format_zap_version(args->version, &version_formatted);
            z_show_error("Version " Z_SV_FMT " is not installed.\n"
                         "       Run: zapup install " Z_SV_FMT " to install it.",
                         Z_SV_FARG(z_strbuf_view(&version_formatted)),
                         Z_SV_FARG(z_strbuf_view(&version_formatted)));
            z_strbuf_destroy(&version_formatted);
            return 1;
        }
        int result = zapup_sync_single_entry(app, entry);
        z_lockfile_unlock(&app->indexlock);
        return result;
    }

    return 0;
}
