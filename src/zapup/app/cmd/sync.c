#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>
#include <zapup/zap/sync.h>

int zapup_sync_single_entry(ZapupApp* app, const ZCliSyncArgs* args, ZVersionIndexEntry* entry) {
    ZapVersion ver = z_version_index_entry_version(entry);

    ZStringBuf version_formatted_buf;
    z_strbuf_init(&version_formatted_buf);
    z_format_zap_version(ver, &version_formatted_buf);

    ZStringView version_formatted = z_strbuf_view(&version_formatted_buf);

    z_show_info("Synchronizing " Z_SV_FMT, Z_SV_FARG(version_formatted));

    bool updated;
    int result = z_sync_zap_repo_with_version(ver, z_strbuf_view(&entry->path), NULL, &updated);
    if (result != 0) {
        const git_error* err = git_error_last();
        z_show_error("Failed to update " Z_SV_FMT ": %s", Z_SV_FARG(version_formatted), err->message);
        z_strbuf_destroy(&version_formatted_buf);
        return 1;
    } else {
        if (updated) {
            const ZapBuildOptions opts = zapup_cli_build_args_to_opts(
                app, &args->build, z_strbuf_view(&entry->path), ver
            );
            ZapBuildResult result = z_cmake_build_zap(&opts);
            if (result.code == Z_ZAP_BUILD_SUCCESS) {
                z_show_info("Successfully updated " Z_SV_FMT, Z_SV_FARG(version_formatted));
                z_strbuf_destroy(&version_formatted_buf);
                return 0;
            } else {
                z_show_error("Failed to compile " Z_SV_FMT, Z_SV_FARG(version_formatted));
                z_strbuf_destroy(&version_formatted_buf);
                return 1;
            }
        } else {
            z_show_info("Already up to date: " Z_SV_FMT, Z_SV_FARG(version_formatted));
            z_strbuf_destroy(&version_formatted_buf);
            return 0;
        }
    }

    return 0;
}

int zapup_exec_sync(ZapupApp* app) {
    int res = zapup_ensure_index_lock(app);
    if (res != 0) return res;

    const ZCliSyncArgs* args = &app->args.cmd_args.sync;
    if (z_zap_ver_is_null(args->version)) {
        for (usize i = 0; i < app->index.len; ++i) {
            int result = zapup_sync_single_entry(app, args, &app->index.entries[i]);
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
        int result = zapup_sync_single_entry(app, args, entry);
        z_lockfile_unlock(&app->indexlock);
        return result;
    }

    return 0;
}
