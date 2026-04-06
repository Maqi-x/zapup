#include <zapup/app.h>

#include <zapup/cli/args.h>
#include <zapup/cli/parse.h>

#include <zapup/clone.h>
#include <zapup/build.h>
#include <zapup/output.h>

#include <git2.h>

void zapup_init(ZapupApp* app) {
    git_libgit2_init();
    z_paths_config_load(&app->paths);
    z_paths_ensure_exists(&app->paths);
    z_version_index_load(&app->index, z_pathbuf_as_view(&app->paths.indexfile));
}

int zapup_do_install(ZapupApp* app) {
    ZResolvableZapVersion v = app->args.cmd_args.install.version;
    git_repository* repo;
    
    ZStringBuf version_formatted;
    z_strbuf_init(&version_formatted);
    z_format_zap_version(v, &version_formatted);

    ZPathBuf out_path;
    z_pathbuf_init_from(&out_path, z_pathbuf_as_view(&app->paths.versions));
    z_pathbuf_join(&out_path, z_strbuf_view(&version_formatted));
    z_pathbuf_sanitize(&out_path);

    z_strbuf_destroy(&version_formatted);
    z_show_info("installing to %.*s...", (int) out_path.len, out_path.data);

    int res = z_clone_zap_repo_with_version(v, z_pathbuf_as_view(&out_path), &repo);
    if (res != 0) {
        const git_error* err = git_error_last();
        z_show_error("%s", err->message);
        z_pathbuf_destroy(&out_path);
        return 1;
    } else {
        const ZCMakeZapBuildOptions opts = {
            .zap_root = z_pathbuf_as_view(&out_path),
            .ver = v,
            .parallel = app->args.cmd_args.install.parallel,
            .max_jobs = app->args.cmd_args.install.max_jobs,
        };
        z_cmake_build_zap(&opts);

        z_pathbuf_destroy(&out_path);
        git_repository_free(repo);
    }
    return 0;
}

int zapup_run(ZapupApp* app, int argc, const char* const* argv) {
    ZCliParseResult err = z_cli_parse_args(argc, argv, &app->args);
    if (err.code != Z_CLI_PARSE_OK) {
        z_show_error("cli parse error: %d", err.code);
        return 1;
    }

    switch (app->args.cmd) {
    case Z_CLI_CMD_INSTALL:
        return zapup_do_install(app);
    case Z_CLI_CMD_UNINSTALL:
        z_show_warn("uninstall: not implemented yet");
        break;
    case Z_CLI_CMD_SYNC:
        z_show_warn("sync: not implemented yet");
        break;
    case Z_CLI_CMD_HELP:
        z_show_warn("help: not implemented yet");
        break;
    case Z_CLI_CMD_UNKNOWN:
        z_show_warn("unknown command");
        break;
    }

    return 0;
}

void zapup_destroy(ZapupApp* app) {
    z_version_index_save(&app->index, z_pathbuf_as_view(&app->paths.indexfile));
    z_paths_config_destroy(&app->paths);
    git_libgit2_shutdown();
}
