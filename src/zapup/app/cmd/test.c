#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>

int zapup_test_single_entry(ZapupApp* app, const ZCliTestArgs* args, ZVersionIndexEntry* entry) {
    (void) app, (void) args; // unused for now
    ZapVersion ver = z_version_index_entry_version(entry);
    return zapup_test_version_at_path(ver, z_strbuf_view(&entry->path));
}

int zapup_exec_test(ZapupApp* app) {
    int res = zapup_ensure_index_lock(app);
    if (res != 0) return res;

    const ZCliTestArgs* args = &app->args.cmd_args.test;
    if (z_zap_ver_is_null(args->version)) {
        for (usize i = 0; i < app->index.len; ++i) {
            int result = zapup_test_single_entry(app, args, &app->index.entries[i]);
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
        int result = zapup_test_single_entry(app, args, entry);
        z_lockfile_unlock(&app->indexlock);
        return result;
    }

    return 0;
}
