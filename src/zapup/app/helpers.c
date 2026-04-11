#include <zapup/app/app.h>
#include <zapup/output.h>

#include <zapup/zap/build.h>
#include <zapup/zap/test.h>

#include <util/fs.h>

int zapup_ensure_index_lock(ZapupApp* app) {
    ZPathView lockpath = z_pathbuf_as_view(&app->paths.indexlock);
    if (!z_lockfile_lock(&app->indexlock, lockpath)) {
        ZPathBuf content;
        z_pathbuf_init(&content);
        z_read_file(lockpath, &content);
        z_show_error("cannot acquire lock. lock file exists (pid: " Z_SV_FMT ")", Z_SV_FARG(content));
        z_pathbuf_destroy(&content);
        return 2;
    }
    return 0;
}

int zapup_get_version_dir_init(ZapupApp* app, ZapVersion ver, ZPathBuf* out_path) {
    ZStringBuf version_formatted;
    z_strbuf_init(&version_formatted);
    z_format_zap_version(ver, &version_formatted);

    z_pathbuf_init_from(out_path, z_pathbuf_as_view(&app->paths.versions));
    z_pathbuf_join(out_path, z_strbuf_view(&version_formatted));
    z_pathbuf_sanitize(out_path);

    z_strbuf_destroy(&version_formatted);
    return 0;
}

int zapup_test_version_at_path(ZapVersion ver, ZPathView path) {
    ZStringBuf version_formatted_buf;
    z_strbuf_init(&version_formatted_buf);
    z_format_zap_version(ver, &version_formatted_buf);

    ZStringView version_formatted = z_strbuf_view(&version_formatted_buf);
    z_show_info("Testing " Z_SV_FMT, Z_SV_FARG(version_formatted));

    bool all_tests_passed;
    if (!z_run_zap_tests(ver, path, &all_tests_passed)) {
        z_show_error("Failed to run tests for " Z_SV_FMT, Z_SV_FARG(version_formatted));
        z_strbuf_destroy(&version_formatted_buf);
        return 1;
    }

    if (all_tests_passed) {
        z_show_info("All tests passed for version " Z_SV_FMT, Z_SV_FARG(version_formatted));
    } else {
        z_show_error("Some tests failed for version " Z_SV_FMT, Z_SV_FARG(version_formatted));
    }

    z_strbuf_destroy(&version_formatted_buf);
    return all_tests_passed ? 0 : 1;
}

ZapBuildOptions zapup_cli_build_args_to_opts(
    ZapupApp* app, const ZCliBuildArgs* args,
    ZStringView zap_root, ZapVersion ver
) {
    return (ZapBuildOptions) {
        .zap_root = zap_root,
        .ver = ver,
        .parallel = args->parallel,
        .max_jobs = args->max_jobs,
        .run_tests = args->run_tests,
        .cc = app->cfg.build.cc,
        .cxx = app->cfg.build.cxx,
    };
}
