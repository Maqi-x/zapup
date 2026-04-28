#include <zapup/app/app.h>
#include <zapup/output.h>

#include <zapup/lsp/nvim.h>

int zapup_exec_init_lsp(ZapupApp* app) {
    ZCliInitLspArgs* args = &app->args.cmd_args.init_lsp;
    
    ZapVersion ver = app->cfg.toolchain.active_version;
    ZVersionIndexEntry* current = z_version_index_find_by_version(&app->index, ver);
    ZPathView zap_root = z_strbuf_view(&current->path);

    if (args->targets & Z_LSP_NVIM) {
        z_show_info("Installing LSP for neovim...");
        if (z_install_zap_lsp_for_nvim(zap_root, args->nvim_mode)) {
            z_show_info("Installed successfully");
        } else {
            z_show_error("Install failed");
        }
    } else if (args->targets & Z_LSP_VSC) {
        z_show_error("Visual studio code is unsupported for now");
        return 1;
    }

    return 0;
}
