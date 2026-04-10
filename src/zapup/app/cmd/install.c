#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/zap/clone.h>
#include <zapup/zap/build.h>
#include <zapup/output.h>

#include <util/fs.h>

#include <stdbool.h>

typedef struct InstallCloneProgressState {
    int last_percent;
    bool printed;
} InstallCloneProgressState;

static void zapup_install_clone_progress(const ZapCloneProgress* progress, void* user_data) {
    if (!progress || !user_data) return;

    InstallCloneProgressState* state = (InstallCloneProgressState*)user_data;
    if (progress->percent < 0 || progress->percent > 100) return;
    if (progress->percent == state->last_percent) return;

    z_show_info_inline("cloning repo: %3d%%...", progress->percent);
    state->last_percent = progress->percent;
    state->printed = true;
}

int zapup_exec_install(ZapupApp* app) {
    int res = zapup_ensure_index_lock(app);
    if (res != 0) return res;

    const ZCliInstallArgs* args = &app->args.cmd_args.install;

    ZapVersion v = args->version;

    ZPathBuf out_path;
    zapup_get_version_dir_init(app, v, &out_path);
    if (res != 0) return res;

    z_show_info("installing to " Z_SV_FMT "...", Z_SV_FARG(out_path));

    InstallCloneProgressState clone_progress = {
        .last_percent = -1,
        .printed = false,
    };
    res = z_clone_zap_repo_with_version_progress(
        v, z_pathbuf_as_view(&out_path), NULL,
        zapup_install_clone_progress, &clone_progress
    );
    if (clone_progress.printed) {
        z_show_info_inline_finish();
    }
    if (res != 0) {
        const git_error* err = git_error_last();
        z_show_error("%s", err->message);
        z_rm_recursive(z_strbuf_view(&out_path));
        z_pathbuf_destroy(&out_path);
        z_lockfile_unlock(&app->indexlock);
        return 1;
    } else {
        z_version_index_add(&app->index, v, z_pathbuf_as_view(&out_path));
        const ZapBuildOptions opts = zapup_cli_build_args_to_opts(
            app, &args->build, z_strbuf_view(&out_path), v
        );
        z_cmake_build_zap(&opts);
        if (args->test) {
            res = zapup_test_version_at_path(v, z_strbuf_view(&out_path));
            if (res != 0) {
                z_lockfile_unlock(&app->indexlock);
                z_pathbuf_destroy(&out_path);
                return res;
            }
        }

        z_lockfile_unlock(&app->indexlock);
        z_pathbuf_destroy(&out_path);
    }
    return 0;
}
