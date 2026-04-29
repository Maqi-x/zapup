#include <zapup/lsp/reinit.h>
#include <zapup/lsp/nvim.h>
#include <zapup/lsp/vsc.h>

#include <zapup/output.h>

bool z_reinit_lsp(ZapVersion version, ZPathView zap_root, ZapLspTarget targets, ZapNvimLspMode nvim_mode) {
    (void) version; // unused for now
    if (targets & Z_LSP_NVIM) {
        z_show_info("Installing LSP for neovim...");
        if (z_install_zap_lsp_for_nvim(zap_root, nvim_mode)) {
            z_show_info("Installed successfully");
        } else {
            z_show_error("Install failed");
            return false;
        }
    }
    if (targets & Z_LSP_VSC) {
        z_show_info("Installing LSP for Visual Studio Code...");
        if (z_install_zap_lsp_for_vsc(zap_root)) {
            z_show_info("Installed successfully");
        } else {
            z_show_error("Install failed");
            return false;
        }
    }
    return true;
}
