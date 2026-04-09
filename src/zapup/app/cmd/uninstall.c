#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>

#include <util/fs.h>

int zapup_exec_uninstall(ZapupApp* app) {
    int res = zapup_ensure_index_lock(app);
    if (res != 0) return res;

    ZapVersion v = app->args.cmd_args.uninstall.version;
    ZVersionIndexEntry* entry = z_version_index_find_by_version(&app->index, v);

    ZStringBuf version_formatted;
    z_strbuf_init(&version_formatted);
    z_format_zap_version(v, &version_formatted);

    if (!entry) {
        z_show_error("version " Z_SV_FMT " is not installed", Z_SV_FARG(version_formatted));
        z_strbuf_destroy(&version_formatted);
        z_lockfile_unlock(&app->indexlock);
        return 1;
    }

    ZPathView path = z_strbuf_view(&entry->path);
    z_show_info("uninstalling " Z_SV_FMT " from " Z_SV_FMT "...",
                    Z_SV_FARG(version_formatted), Z_SV_FARG(path));

    if (z_file_exists(path)) {
        if (!z_rm_recursive(path)) {
            z_show_error("failed to remove directory " Z_SV_FMT, Z_SV_FARG(path));
            z_lockfile_unlock(&app->indexlock);
            return 1;
        }
    }

    z_version_index_remove_by_version(&app->index, v);

    z_strbuf_destroy(&version_formatted);
    z_lockfile_unlock(&app->indexlock);
    z_show_info("uninstalled successfully");
    return 0;
}
