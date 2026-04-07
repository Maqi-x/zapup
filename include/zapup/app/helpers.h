#include <zapup/app/app.h>

#include <zapup/zap/build.h>

int zapup_ensure_index_lock(ZapupApp* app);
int zapup_get_version_dir_init(ZapupApp* app, ZResolvableZapVersion ver, ZPathBuf* out_path);

ZapBuildOptions zapup_cli_build_args_to_opts(
    ZapupApp* app, const ZCliBuildArgs* args,
    ZStringView zap_root, ZResolvableZapVersion ver
);
