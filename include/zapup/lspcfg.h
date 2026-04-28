#pragma once

#include <zapup/zap/lsp.h>

#include <util/pathview.h>
#include <util/strbuf.h>
#include <defs/sv.h>

typedef struct ZLspConfig {
    ZapLspTarget targets;     // zero or more
    ZapNvimLspMode nvim_mode; // unknown if !(targets & Z_LSP_NVIM)
} ZLspConfig;

void z_lspcfg_from_json(ZLspConfig* cfg, ZStringView json);
bool z_lspcfg_to_json(const ZLspConfig* cfg, ZStringBuf* out);

bool z_lspcfg_load(ZLspConfig* cfg, ZPathView path);
bool z_lspcfg_save(const ZLspConfig* cfg, ZPathView path);
