#pragma once

#include <zapup/zap/version.h>
#include <zapup/zap/lsp.h>

#include <util/pathview.h>

bool z_reinit_lsp(ZapVersion ver, ZPathView zap_root, ZapLspTarget targets, ZapNvimLspMode nvim_mode);
