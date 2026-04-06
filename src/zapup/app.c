#include <zapup/app.h>

#include <zapup/cli/args.h>
#include <zapup/cli/parse.h>

#include <zapup/clone.h>
#include <zapup/output.h>

#include <git2.h>

void zapup_init(ZapupApp* app) {
    git_libgit2_init();
    z_paths_config_load(&app->paths);
    z_paths_ensure_exists(&app->paths);
    z_version_index_load(&app->index, z_pathbuf_as_view(&app->paths.indexfile));
}

int zapup_run(ZapupApp* app, int argc, const char* const* argv) {
    ZCliParseResult err = z_cli_parse_args(argc, argv, &app->args);
    if (err.code != Z_CLI_PARSE_OK) {
        z_show_error("cli parse error: %d", err.code);
        return 1;
    }

    switch (app->args.cmd) {
    case Z_CLI_CMD_INSTALL: {
        z_show_info("installing...");
        ZResolvableZapVersion v = app->args.cmd_args.install.version;
        git_repository* repo;
        int res = z_clone_zap_repo_with_version(v, Z_PV("./out-repo"), &repo);
        if (res != 0) {
            const git_error* err = git_error_last();
            z_show_error("%s", err->message);
            return 1;
        } else {
            git_repository_free(repo);
        }
        break;
    }
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
