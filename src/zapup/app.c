#include <zapup/app.h>

#include <zapup/cli/args.h>
#include <zapup/cli/parse.h>

#include <zapup/clone.h>
#include <zapup/build.h>
#include <zapup/output.h>

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
    z_config_load(&app->cfg, z_pathbuf_as_view(&app->paths.cfgfile));
}

int zapup_do_install(ZapupApp* app) {
    ZPathView lockpath = z_pathbuf_as_view(&app->paths.indexlock);
    if (!z_lockfile_lock(&app->indexlock, lockpath)) {
        ZPathBuf content;
        z_pathbuf_init(&content);
        z_read_file(lockpath, &content);
        z_show_error("cannot acquire lock. lock file exists (pid: " Z_SV_FMT ")", Z_SV_FARG(content));
        z_pathbuf_destroy(&content);
        return 2;
    }

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
    z_show_info("installing to " Z_SV_FMT "...", Z_SV_FARG(out_path));

    int res = z_clone_zap_repo_with_version(v, z_pathbuf_as_view(&out_path), &repo);
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

int zapup_do_uninstall(ZapupApp* app) {
    ZPathView lockpath = z_pathbuf_as_view(&app->paths.indexlock);
    if (!z_lockfile_lock(&app->indexlock, lockpath)) {
        ZPathBuf content;
        z_pathbuf_init(&content);
        z_read_file(lockpath, &content);
        z_show_error("cannot acquire lock. lock file exists (pid: " Z_SV_FMT ")", Z_SV_FARG(content));
        z_pathbuf_destroy(&content);
        return 2;
    }

    ZResolvableZapVersion v = app->args.cmd_args.uninstall.version;
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
        return zapup_do_uninstall(app);
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
    z_version_index_free(&app->index);
    z_config_save(&app->cfg, z_pathbuf_as_view(&app->paths.cfgfile));
    z_config_free(&app->cfg);
    z_lockfile_destroy(&app->indexlock);
    z_paths_config_destroy(&app->paths);
    git_libgit2_shutdown();
}
