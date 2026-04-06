#include <zapup/build.h>

#include <util/cmd.h>
#include <util/fs.h>

ZCMakeZapBuildResult z_cmake_build_zap(const ZCMakeZapBuildOptions* opts) {
    ZPathBuf build_dir_buf;
    z_pathbuf_init_from(&build_dir_buf, opts->zap_root);
    z_pathbuf_join(&build_dir_buf, Z_PV("build"));

    ZPathView build_dir = z_pathbuf_as_view(&build_dir_buf);
    if (!z_mkdir_all(build_dir)) {
        return (ZCMakeZapBuildResult) { .code = Z_ZAP_BUILD_FS_ERROR };
    }

    ZStringBuf build_type_arg;
    z_strbuf_init_from(&build_type_arg, Z_SV("-DCMAKE_BUILD_TYPE="));
    z_strbuf_append(&build_type_arg, opts->ver.build == Z_BUILD_DEBUG ? Z_SV("Debug") : Z_SV("Release"));

    ZStringBuf capture;
    z_strbuf_init(&capture);
    ZCmdRunResult result;

    ZCommand configure_cmd = {0};
    configure_cmd.argv = Z_STRING_VIEWS(
        Z_SV("cmake"), opts->zap_root,
        z_strbuf_view(&build_type_arg),
        Z_SV("-DZAP_BUILD_REFERENCE=OFF"),
        Z_SV("-DCMAKE_CXX_FLAGS=-w"),
    );
    configure_cmd.cwd = build_dir;
    configure_cmd.capture_stdout = &capture;
    result = z_cmd_run(&configure_cmd);
    if (result.status != Z_CMD_OK) {
        return (ZCMakeZapBuildResult) { .code = Z_ZAP_BUILD_CONFIGURE_ERR };
    }

    ZCommand compile_cmd = {0};
    compile_cmd.argv = Z_STRING_VIEWS(
        Z_SV("cmake"), Z_SV("--build"), build_dir,
    );
    compile_cmd.cwd = build_dir;
    result = z_cmd_run(&compile_cmd);
    if (result.status != Z_CMD_OK) {
        return (ZCMakeZapBuildResult) { .code = Z_ZAP_BUILD_COMPILATION_ERR };
    }
    
    return (ZCMakeZapBuildResult) { .code = Z_ZAP_BUILD_SUCCESS };
}
