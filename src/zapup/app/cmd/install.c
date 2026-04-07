#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/zap/clone.h>
#include <zapup/zap/build.h>
#include <zapup/output.h>

#include <util/fs.h>

int zapup_exec_install(ZapupApp* app) {
    int res = zapup_ensure_index_lock(app);
    if (res != 0) return res;

    ZResolvableZapVersion v = app->args.cmd_args.install.version;
    git_repository* repo;

    ZPathBuf out_path;
    zapup_get_version_dir_init(app, v, &out_path);
    if (res != 0) return res;

    z_show_info("installing to " Z_SV_FMT "...", Z_SV_FARG(out_path));

    res = z_clone_zap_repo_with_version(v, z_pathbuf_as_view(&out_path), &repo);
    if (res != 0) {
        const git_error* err = git_error_last();
        z_show_error("%s", err->message);
        z_pathbuf_destroy(&out_path);
        z_lockfile_unlock(&app->indexlock);
        return 1;
    } else {
        z_version_index_add(&app->index, v, z_pathbuf_as_view(&out_path));
        const ZCMakeZapBuildOptions opts = {
            .zap_root = z_pathbuf_as_view(&out_path),
            .ver = v,
            .parallel = app->args.cmd_args.install.parallel,
            .max_jobs = app->args.cmd_args.install.max_jobs,
            .cc = app->cfg.build.cc,
            .cxx = app->cfg.build.cxx,
        };
        z_cmake_build_zap(&opts);

        z_lockfile_unlock(&app->indexlock);
        z_pathbuf_destroy(&out_path);
        git_repository_free(repo);
    }
    return 0;
}
