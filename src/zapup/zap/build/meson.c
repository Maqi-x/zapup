#include <zapup/zap/build.h>
#include <zapup/zap/test.h>

#include <util/strconv.h>
#include <util/cmd.h>
#include <util/fs.h>

ZapBuildResult z_meson_build_zap(const ZapBuildOptions* opts) {
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
    z_strbuf_init_from(&build_type_arg, Z_SV("--buildtype="));
    z_strbuf_append(&build_type_arg, opts->ver.build == Z_BUILD_DEBUG ? Z_SV("debug") : Z_SV("release"));

    ZStringBuf cc_env;
    z_strbuf_init(&cc_env);
    if (opts->cc.len > 0) {
        z_strbuf_append(&cc_env, Z_SV("CC="));
        z_strbuf_append(&cc_env, opts->cc);
    }

    ZStringBuf cxx_env;
    z_strbuf_init(&cxx_env);
    if (opts->cxx.len > 0) {
        z_strbuf_append(&cxx_env, Z_SV("CXX="));
        z_strbuf_append(&cxx_env, opts->cxx);
    }

    ZStringBuf capture;
    z_strbuf_init(&capture);

    ZStringView env_vars[2];
    usize env_count = 0;
    if (cc_env.len > 0) {
        env_vars[env_count++] = z_strbuf_view(&cc_env);
    }
    if (cxx_env.len > 0) {
        env_vars[env_count++] = z_strbuf_view(&cxx_env);
    }

    ZStringView config_argv[8];
    usize config_argc = 0;
    config_argv[config_argc++] = Z_SV("meson");
    config_argv[config_argc++] = Z_SV("setup");
    config_argv[config_argc++] = build_dir;
    config_argv[config_argc++] = opts->zap_root;
    config_argv[config_argc++] = z_strbuf_view(&build_type_arg);

    ZCommand configure_cmd = {
        .cwd = opts->zap_root,
        .argv = { .data = config_argv, .count = config_argc },
        .env = { .data = env_vars, .count = env_count },
#ifdef ZAPUP_RELEASE
            &capture
#else
            NULL
#endif
    };

    ZCmdRunResult configure_result = z_cmd_run(&configure_cmd);
    if (configure_result.status != Z_CMD_OK || configure_result.exit_code != 0) {
        final_res.code = Z_ZAP_BUILD_CONFIGURE_ERR;
        goto cleanup;
    }

    ZStringBuf parallel_flag;
    z_strbuf_init(&parallel_flag);
    if (opts->parallel) {
        z_strbuf_append(&parallel_flag, Z_SV("-j"));
        if (opts->max_jobs != 0) {
            z_format_int(opts->max_jobs, &parallel_flag);
        }
    }

    ZStringView build_argv[8];
    usize build_argc = 0;
    build_argv[build_argc++] = Z_SV("meson");
    build_argv[build_argc++] = Z_SV("compile");
    build_argv[build_argc++] = Z_SV("-C");
    build_argv[build_argc++] = build_dir;
    if (opts->parallel) {
        build_argv[build_argc++] = z_strbuf_view(&parallel_flag);
    }

    ZCommand compile_cmd = {
        .cwd = opts->zap_root,
        .argv = { .data = build_argv, .count = build_argc }
    };

    ZCmdRunResult compile_result = z_cmd_run(&compile_cmd);
    if (compile_result.status != Z_CMD_OK || compile_result.exit_code != 0) {
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
    z_pathbuf_destroy(&build_dir_buf);
    z_strbuf_destroy(&build_type_arg);
    z_strbuf_destroy(&parallel_flag);
    z_strbuf_destroy(&capture);
    z_strbuf_destroy(&cc_env);
    z_strbuf_destroy(&cxx_env);

    return final_res;
}
