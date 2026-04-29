#include <zapup/app/switch.h>
#include <zapup/lsp/reinit.h>
#include <zapup/output.h>

#include <util/strbuf.h>

int zapup_switch_to_version(ZapupApp* app, ZapVersion version, bool local) {
    ZConfig* cfg = local ? &app->local_cfg : &app->global_cfg;
    ZapVersion old_ver = cfg->toolchain.active_version;

    ZStringBuf new_version_formatted;
    z_strbuf_init(&new_version_formatted);
    z_format_zap_version(version, &new_version_formatted);

    if (!z_version_index_find_by_version(&app->index, version)) {
        z_show_error("Version " Z_SV_FMT " not found", Z_SV_FARG(z_strbuf_view(&new_version_formatted)));
        z_strbuf_destroy(&new_version_formatted);
        return 1;
    }

    cfg->toolchain.active_version = version;
    if (local) app->used_local_cfg = true;

    if (local || !app->used_local_cfg) {
        app->cfg.toolchain.active_version = version;
    }

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

    ZVersionIndexEntry* current = z_version_index_find_by_version(&app->index, version);
    ZPathView zap_root = z_strbuf_view(&current->path);
    z_reinit_lsp(version, zap_root, app->lsp_cfg.targets, app->lsp_cfg.nvim_mode);

    z_strbuf_destroy(&new_version_formatted);
    return 0;
}
