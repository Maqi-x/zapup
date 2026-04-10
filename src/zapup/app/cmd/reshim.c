#include <zapup/app/app.h>
#include <zapup/app/helpers.h>

#include <zapup/output.h>
#include <zapup/shims.h>

#include <util/fs.h>
#include <util/arr-len.h>

int zapup_reshim_single(ZapupApp* app, ZapToolchainElement tool, ZStringView self_bin) {
    ZStringView name = z_format_zap_toolchain_element(tool);

    ZPathBuf path;
    z_pathbuf_init_from(&path, z_pathbuf_as_view(&app->paths.shims));
    z_pathbuf_join(&path, name);

    ZStringBuf shim_buf;
    z_strbuf_init(&shim_buf);

    z_show_info("Generating shim for " Z_SV_FMT "...", Z_SV_FARG(name));

    int result = 0;

    bool ok = z_generate_native_shim_for(self_bin, tool, &shim_buf);
    if (!ok) {
        z_show_error("Failed to generate shim");
        result = 1; goto cleanup;
    }

    ok = z_write_file(z_pathbuf_as_view(&path), z_strbuf_view(&shim_buf));
    if (!ok) {
        z_show_error("Failed to save shim to file");
        result = 1; goto cleanup;
    }

    ok = z_set_executable(z_pathbuf_as_view(&path), true);
    if (!ok) {
        z_show_error("Failed to make shim executable");
        result = 1; goto cleanup;
    }

cleanup:
    z_pathbuf_destroy(&path);
    z_strbuf_destroy(&shim_buf);
    return result;
}

int zapup_exec_reshim(ZapupApp* app) {
    ZStringBuf self_bin_path;
    z_strbuf_init(&self_bin_path);
    if (!z_get_self_executable(&self_bin_path)) {
        z_show_error("Failed to get self executable");
        return 1;
    }

    const ZCliReshimArgs* args = &app->args.cmd_args.reshim;
    if (args->tool != Z_TOOLCHAIN_ELEMENT_UNKNOWN) {
        int result = zapup_reshim_single(app, args->tool, z_strbuf_view(&self_bin_path));
        if (result != 0) return result;
    } else {
        ZapToolchainElement tools[] = {
            Z_TOOLCHAIN_ELEMENT_ZAPC, Z_TOOLCHAIN_ELEMENT_ZAP_LSP
        };
        for (usize i = 0; i < Z_ARRAY_LEN(tools); ++i) {
            int result = zapup_reshim_single(app, tools[i], z_strbuf_view(&self_bin_path));
            if (result != 0) return result;
        }
    }
    return 0;
}
