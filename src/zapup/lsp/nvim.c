#include <zapup/lsp/nvim.h>

#include <util/pathview.h>
#include <util/pathbuf.h>

#include <util/cmd.h>

bool z_install_zap_lsp_for_nvim(ZPathView zap_root, ZapNvimLspMode mode) {
    ZPathBuf lsp_root_buf;
    z_pathbuf_init_from(&lsp_root_buf, zap_root);
    z_pathbuf_join(&lsp_root_buf, Z_PV("src/lsp/nvim"));

    ZPathView lsp_root = z_pathbuf_as_view(&lsp_root_buf);

    ZPathBuf install_script;
    z_pathbuf_init_from(&install_script, lsp_root);
    switch (mode) {
    case Z_NVIM_LSP_INIT_LUA:
        z_pathbuf_join(&install_script, Z_PV("install-init.lua.sh"));
        break;
    case Z_NVIM_LSP_LAZY:
        z_pathbuf_join(&install_script, Z_PV("install-lazy.sh"));
        break;
    case Z_NVIM_LSP_UNKNOWN:
        return false;
    }

    ZStringBuf ignore;
    z_strbuf_init(&ignore);

    ZCommand install_cmd = {
        .cwd = lsp_root,
        .argv = Z_STRING_VIEWS(
            z_pathbuf_as_view(&install_script),
        ),
        .capture_stdout = &ignore,
    };
    ZCmdRunResult result = z_cmd_run(&install_cmd);

    z_strbuf_destroy(&ignore);
    z_pathbuf_destroy(&lsp_root_buf);
    z_pathbuf_destroy(&install_script);

    if (result.status != Z_CMD_OK || result.exit_code != 0) {
        return false;
    }
    return true;
}

