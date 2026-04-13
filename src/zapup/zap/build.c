#include <zapup/zap/build.h>
#include <zapup/zap/test.h>

#include <util/strconv.h>
#include <util/cmd.h>
#include <util/fs.h>

ZapBuildResult z_cmake_build_zap(const ZapBuildOptions* opts) {
    ZapBuildResult final_res = { .code = Z_ZAP_BUILD_SUCCESS };

    ZPathBuf build_dir_buf;
    z_pathbuf_init_from(&build_dir_buf, opts->zap_root);
    z_pathbuf_join(&build_dir_buf, Z_PV("build"));

    ZPathView build_dir = z_pathbuf_as_view(&build_dir_buf);
    if (!z_mkdir_all(build_dir)) {
        z_pathbuf_destroy(&build_dir_buf);
        return (ZapBuildResult) { .code = Z_ZAP_BUILD_FS_ERROR };
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

    ZStringBuf cc_arg;
    z_strbuf_init(&cc_arg);
    if (opts->cc.len > 0) {
        z_strbuf_append(&cc_arg, Z_SV("-DCMAKE_C_COMPILER="));
        z_strbuf_append(&cc_arg, opts->cc);
    }

    ZStringBuf cxx_arg;
    z_strbuf_init(&cxx_arg);
    if (opts->cxx.len > 0) {
        z_strbuf_append(&cxx_arg, Z_SV("-DCMAKE_CXX_COMPILER="));
        z_strbuf_append(&cxx_arg, opts->cxx);
    }

    ZStringView config_argv[12];
    usize config_argc = 0;
    config_argv[config_argc++] = Z_SV("cmake");
    config_argv[config_argc++] = opts->zap_root;
    config_argv[config_argc++] = z_strbuf_view(&build_type_arg);
    config_argv[config_argc++] = Z_SV("-DZAP_BUILD_REFERENCE=OFF");
    config_argv[config_argc++] = Z_SV("-DCMAKE_CXX_FLAGS=-w");

    if (cc_arg.len > 0) {
        config_argv[config_argc++] = z_strbuf_view(&cc_arg);
    }
    if (cxx_arg.len > 0) {
        config_argv[config_argc++] = z_strbuf_view(&cxx_arg);
    }

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

    if (opts->run_tests){
        if (!z_run_zap_tests(opts->ver, opts->zap_root, NULL)) {
            final_res.code = Z_ZAP_BUILD_TESTS_ERR;
            goto cleanup;
        }
    }

cleanup:
    z_strbuf_destroy(&build_type_arg);
    z_strbuf_destroy(&parallel_flag);
    z_strbuf_destroy(&capture);
    z_strbuf_destroy(&cc_arg);
    z_strbuf_destroy(&cxx_arg);
    z_pathbuf_destroy(&build_dir_buf);

    return final_res;
}
