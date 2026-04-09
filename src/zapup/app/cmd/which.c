#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <util/macros.h>
#include <zapup/output.h>

int zapup_exec_which(ZapupApp* app) {
    const ZCliWhichArgs* args = &app->args.cmd_args.which;

    const ZapVersion ver =
        !z_zap_ver_is_null(args->version)
        ? args->version
        : app->cfg.toolchain.active_version;

    ZVersionIndexEntry* entry = z_version_index_find_by_version(&app->index, ver);

    ZStringBuf version_formatted;
    z_strbuf_init(&version_formatted);
    z_format_zap_version(ver, &version_formatted);

    if (!entry) {
        z_show_error("version " Z_SV_FMT " is not installed", Z_SV_FARG(version_formatted));
        z_strbuf_destroy(&version_formatted);
        return 1;
    }

    ZPathBuf path;
    z_pathbuf_init_from(&path, z_pathbuf_as_view(&entry->path));
    z_pathbuf_join(&path, Z_PV("build"));

    switch (args->tool) {
    case Z_TOOLCHAIN_ELEMENT_ZAPC:
        z_pathbuf_join(&path, Z_PV("zapc"));
        break;
    case Z_TOOLCHAIN_ELEMENT_ZAP_LSP:
        z_pathbuf_join(&path, Z_PV("zap-lsp"));
        break;
    case Z_TOOLCHAIN_ELEMENT_UNKNOWN:
        Z_UNREACHABLE("tool should not be unknown");
    }

    z_sv_print(z_pathbuf_as_view(&path), stdout);

    z_pathbuf_destroy(&path);
    z_strbuf_destroy(&version_formatted);
    return 0;
}
