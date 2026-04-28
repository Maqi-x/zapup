#include <zapup/app/app.h>
#include <zapup/output.h>

#include <zapup/lsp/reinit.h>

int zapup_exec_init_lsp(ZapupApp* app) {
    ZCliInitLspArgs* args = &app->args.cmd_args.init_lsp;    
    ZapVersion ver = app->cfg.toolchain.active_version;

    app->lsp_cfg.targets = args->targets;
    app->lsp_cfg.nvim_mode = args->nvim_mode;

    ZVersionIndexEntry* current = z_version_index_find_by_version(&app->index, ver);
    ZPathView zap_root = z_strbuf_view(&current->path);

    return z_reinit_lsp(ver, zap_root, args->targets, args->nvim_mode) ? 0 : 1;
}
