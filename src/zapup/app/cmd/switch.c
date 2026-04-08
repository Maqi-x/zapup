#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>

int zapup_exec_switch(ZapupApp* app) {
    const ZCliSwitchArgs* args = &app->args.cmd_args.switch_;
   
    ZResolvableZapVersion old_ver = app->cfg.toolchain.active_version;

    ZStringBuf new_version_formatted;
    z_strbuf_init(&new_version_formatted);
    z_format_zap_version(args->version, &new_version_formatted);
   
    if (!z_version_index_find_by_version(&app->index, args->version)) {
        z_show_error("Version " Z_SV_FMT " not found", Z_SV_FARG(z_strbuf_view(&new_version_formatted)));
        z_strbuf_destroy(&new_version_formatted);
        return 1;
    }

    app->cfg.toolchain.active_version = args->version;

    if (!z_zap_ver_is_null(old_ver)) {
        ZStringBuf old_version_formatted;
        z_strbuf_init(&old_version_formatted);
        z_format_zap_version(old_ver, &old_version_formatted);
        z_show_info(
            "Switched from " Z_SV_FMT " to " Z_SV_FMT,
            Z_SV_FARG(z_strbuf_view(&old_version_formatted)),
            Z_SV_FARG(z_strbuf_view(&new_version_formatted))
        );
        z_strbuf_destroy(&old_version_formatted);
    } else {
        z_show_info("Switched to " Z_SV_FMT, Z_SV_FARG(z_strbuf_view(&new_version_formatted)));
    }

    z_strbuf_destroy(&new_version_formatted);
    return 0;
}
