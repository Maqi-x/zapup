#include <zapup/app/app.h>
#include <zapup/app/cmds.h>
#include <zapup/app/helpers.h>

#include <zapup/index.h>
#include <zapup/storage.h>

#include <zapup/cli/args.h>
#include <zapup/cli/parse.h>

#include <zapup/output.h>
#include <zapup/help.h>

#include <util/fs.h>
#include <git2.h>

void zapup_init(ZapupApp* app) {
    git_libgit2_init();
    z_paths_config_load(&app->paths);
    z_paths_ensure_exists(&app->paths);
    z_version_index_init(&app->index);
    z_version_index_load(&app->index, z_pathbuf_as_view(&app->paths.indexfile));
    z_lockfile_init(&app->indexlock);
    z_config_init(&app->cfg);
}

int zapup_load_config(ZapupApp* app) {
    if (!z_config_load(&app->cfg, z_pathbuf_as_view(&app->paths.cfgfile))) {
        return 1;
    }

    if (!app->args.global_args.ignore_local && z_file_exists(Z_PV("./.zapup.json"))) {
        if (!z_config_merge_load(&app->cfg, Z_PV("./.zapup.json"))) {
            return 1;
        }
    }

    z_show_debug("CC:  " Z_SV_FMT, Z_SV_FARG(app->cfg.build.cc));
    z_show_debug("CXX: " Z_SV_FMT, Z_SV_FARG(app->cfg.build.cxx));
    return 0;
}

int zapup_run(ZapupApp* app, int argc, const char* const* argv) {
    ZCliParseResult err = z_cli_parse_args(argc, argv, &app->args);
    if (err.code != Z_CLI_PARSE_OK) {
        z_parse_result_print(&err, Z_OUTPUT_ERROR);
        ZHelpInfo info = zapup_get_help();
        zapup_show_help(&info, app->args.cmd, stderr);
        return 1;
    }

    int result = zapup_load_config(app);
    if (result != 0) {
        return result;
    }

    switch (app->args.cmd) {
    case Z_CLI_CMD_INSTALL:
        return zapup_exec_install(app);
    case Z_CLI_CMD_UNINSTALL:
        return zapup_exec_uninstall(app);
    case Z_CLI_CMD_RESHIM:
        return zapup_exec_reshim(app);
    case Z_CLI_CMD_SWITCH:
        return zapup_exec_switch(app);
    case Z_CLI_CMD_WHICH:
        return zapup_exec_which(app);
    case Z_CLI_CMD_LIST:
        return zapup_exec_list(app);
    case Z_CLI_CMD_SHOW:
        return zapup_exec_show(app);
    case Z_CLI_CMD_TEST:
        return zapup_exec_test(app);
    case Z_CLI_CMD_SYNC:
        return zapup_exec_sync(app);
    case Z_CLI_CMD_HELP:
        return zapup_exec_help(app);
    case Z_CLI_CMD_UNKNOWN:
        z_show_warn("unknown command");
        break;
    }

    return 0;
}

void zapup_destroy(ZapupApp* app) {
    z_version_index_save(&app->index, z_pathbuf_as_view(&app->paths.indexfile));
    z_version_index_free(&app->index);
    z_config_save(&app->cfg, z_pathbuf_as_view(&app->paths.cfgfile));
    z_config_free(&app->cfg);
    z_lockfile_destroy(&app->indexlock);
    z_paths_config_destroy(&app->paths);
    git_libgit2_shutdown();
}
