#include <zapup/build.h>

#include <util/strconv.h>
#include <util/cmd.h>
#include <util/fs.h>

ZCMakeZapBuildResult z_cmake_build_zap(const ZCMakeZapBuildOptions* opts) {
    ZCMakeZapBuildResult final_res = { .code = Z_ZAP_BUILD_SUCCESS };
    
    ZPathBuf build_dir_buf;
    z_pathbuf_init_from(&build_dir_buf, opts->zap_root);
    z_pathbuf_join(&build_dir_buf, Z_PV("build"));

    ZPathView build_dir = z_pathbuf_as_view(&build_dir_buf);
    if (!z_mkdir_all(build_dir)) {
        z_pathbuf_destroy(&build_dir_buf);
        return (ZCMakeZapBuildResult) { .code = Z_ZAP_BUILD_FS_ERROR };
    }

    ZStringBuf build_type_arg;
    z_strbuf_init_from(&build_type_arg, Z_SV("-DCMAKE_BUILD_TYPE="));
    z_strbuf_append(&build_type_arg, opts->ver.build == Z_BUILD_DEBUG ? Z_SV("Debug") : Z_SV("Release"));

    ZStringBuf parallel_flag;
    z_strbuf_init(&parallel_flag);
    if (opts->parallel) {
        z_strbuf_append(&parallel_flag, Z_SV("-j"));
        if (opts->max_jobs != 0) {
            z_format_int(opts->max_jobs, &parallel_flag);
        }
    }

    ZStringBuf capture;
    z_strbuf_init(&capture);

    ZStringView config_argv[8];
    usize config_argc = 0;
    config_argv[config_argc++] = Z_SV("cmake");
    config_argv[config_argc++] = opts->zap_root;
    config_argv[config_argc++] = z_strbuf_view(&build_type_arg);
    config_argv[config_argc++] = Z_SV("-DZAP_BUILD_REFERENCE=OFF");
    config_argv[config_argc++] = Z_SV("-DCMAKE_CXX_FLAGS=-w");

    ZCommand configure_cmd = {
        .cwd = build_dir,
        .argv = { .data = config_argv, .count = config_argc },
        .capture_stdout = &capture
    };

    if (z_cmd_run(&configure_cmd).status != Z_CMD_OK) {
        final_res.code = Z_ZAP_BUILD_CONFIGURE_ERR;
        goto cleanup;
    }

    ZStringView build_argv[8];
    usize build_argc = 0;
    build_argv[build_argc++] = Z_SV("cmake");
    build_argv[build_argc++] = Z_SV("--build");
    build_argv[build_argc++] = build_dir;
    if (opts->parallel) {
        build_argv[build_argc++] = z_strbuf_view(&parallel_flag);
    }

    ZCommand compile_cmd = {
        .cwd = build_dir,
        .argv = { .data = build_argv, .count = build_argc }
    };

    if (z_cmd_run(&compile_cmd).status != Z_CMD_OK) {
        final_res.code = Z_ZAP_BUILD_COMPILATION_ERR;
        goto cleanup;
    }

cleanup:
    z_strbuf_destroy(&build_type_arg);
    z_strbuf_destroy(&parallel_flag);
    z_strbuf_destroy(&capture);
    z_pathbuf_destroy(&build_dir_buf);

    return final_res;
}
