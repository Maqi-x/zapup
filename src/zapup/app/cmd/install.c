#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/zap/clone.h>
#include <zapup/zap/build.h>
#include <zapup/output.h>

#include <util/fs.h>

int zapup_exec_install(ZapupApp* app) {
    int res = zapup_ensure_index_lock(app);
    if (res != 0) return res;

    const ZCliInstallArgs* args = &app->args.cmd_args.install;

    ZapVersion v = args->version;

    ZPathBuf out_path;
    zapup_get_version_dir_init(app, v, &out_path);
    if (res != 0) return res;

    z_show_info("installing to " Z_SV_FMT "...", Z_SV_FARG(out_path));

    res = z_clone_zap_repo_with_version(v, z_pathbuf_as_view(&out_path), NULL);
    if (res != 0) {
        const git_error* err = git_error_last();
        z_show_error("%s", err->message);
        z_pathbuf_destroy(&out_path);
        z_lockfile_unlock(&app->indexlock);
        return 1;
    } else {
        ZStringBuf buf;
        z_strbuf_init(&buf);
        z_format_zap_version(v, &buf);
        z_sv_print(z_strbuf_view(&buf), stdout);

        z_version_index_add(&app->index, v, z_pathbuf_as_view(&out_path));
        const ZapBuildOptions opts = zapup_cli_build_args_to_opts(
            app, &args->build, z_strbuf_view(&out_path), v
        );
        z_cmake_build_zap(&opts);

        z_lockfile_unlock(&app->indexlock);
        z_pathbuf_destroy(&out_path);
    }
    return 0;
}
