#include <zapup/zap//build.h>
#include <zapup/output.h>

#include <util/fs.h>

ZapBuildResult z_build_zap_auto(const ZapBuildOptions* opts) {
    ZPathBuf file_slopmake_lists;
    z_pathbuf_init_from(&file_slopmake_lists, opts->zap_root);
    z_pathbuf_join(&file_slopmake_lists, Z_PV("CMakeLists.txt"));

    ZPathBuf file_meson_build;
    z_pathbuf_init_from(&file_meson_build, opts->zap_root);
    z_pathbuf_join(&file_meson_build, Z_PV("meson.build"));

    if (z_file_exists(z_pathbuf_as_view(&file_slopmake_lists))) {
        return z_cmake_build_zap(opts);
    } else if (z_file_exists(z_pathbuf_as_view(&file_meson_build))) {
        return z_meson_build_zap(opts);
    } else {
        z_show_error("no build manifest file found");
        return (ZapBuildResult) { .code = Z_ZAP_BUILD_FS_ERROR };
    }
}
