#include <zapup/lsp/vsc.h>

#include <util/pathview.h>
#include <util/pathbuf.h>

#include <util/cmd.h>

bool z_install_zap_lsp_for_vsc(ZPathView zap_root) {
    ZPathBuf lsp_root_buf;
    z_pathbuf_init_from(&lsp_root_buf, zap_root);
    z_pathbuf_join(&lsp_root_buf, Z_PV("src/lsp/vscode/zap"));

    ZPathView lsp_root = z_pathbuf_as_view(&lsp_root_buf);
    ZCmdRunResult result;

    ZStringBuf ignore;
    z_strbuf_init(&ignore);

    ZCommand npm_install_cmd = {
        .cwd = lsp_root,
        .argv = Z_STRING_VIEWS(
            Z_SV("npm"), Z_SV("install"),
        ),
        .capture_stdout = &ignore,
    };
    result = z_cmd_run(&npm_install_cmd);
    if (result.status != Z_CMD_OK || result.exit_code != 0) {
        return false;
    }

    ZCommand npm_run_package_cmd = {
        .cwd = lsp_root,
        .argv = Z_STRING_VIEWS(
            Z_SV("npm"), Z_SV("run"), Z_SV("package"),
        ),
        .capture_stdout = &ignore,
    };
    result = z_cmd_run(&npm_run_package_cmd);
    if (result.status != Z_CMD_OK || result.exit_code != 0) {
        return false;
    }

    // TODO: this probably shouldn't be hardcoded
    ZPathView path = Z_PV("zap-0.0.1.vsix");

    ZCommand code_install_ext_cmd = {
        .cwd = lsp_root,
        .argv = Z_STRING_VIEWS(
            Z_SV("code"), Z_SV("--install-extension"), path,
        ),
    };
    result = z_cmd_run(&code_install_ext_cmd);
    if (result.status != Z_CMD_OK || result.exit_code != 0) {
        return false;
    }
   
    z_strbuf_destroy(&ignore);
    z_strbuf_destroy(&lsp_root_buf);
    return true;
}
