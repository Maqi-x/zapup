#pragma once

#include <defs/sv.h>

typedef enum ZapLspTarget {
    Z_LSP_UNKNOWN = 0,
    Z_LSP_NVIM    = 1 << 0,
    Z_LSP_VSC     = 1 << 1,
} ZapLspTarget;

typedef enum ZapNvimLspMode {
    Z_NVIM_LSP_UNKNOWN  = 0,
    Z_NVIM_LSP_INIT_LUA = 1 << 0,
    Z_NVIM_LSP_LAZY     = 1 << 1,
} ZapNvimLspMode;

static inline ZStringView z_format_zap_lsp_target(ZapLspTarget target) {
    switch (target) {
    case Z_LSP_NVIM:
        return Z_SV("nvim");
    case Z_LSP_VSC:
        return Z_SV("vsc");
    case Z_LSP_UNKNOWN:
        return Z_SV_NULL;
    }
}

static inline ZapLspTarget z_parse_zap_lsp_target(ZStringView str) {
    if (z_sv_eql(str, Z_SV("nvim")) || z_sv_eql(str, Z_SV("neovim"))) {
        return Z_LSP_NVIM;
    } else if (z_sv_eql(str, Z_SV("vsc")) || z_sv_eql(str, Z_SV("code"))) {
        return Z_LSP_VSC;
    } else{
        return Z_LSP_UNKNOWN;
    }
}

static inline ZStringView z_format_nvim_lsp_mode(ZapNvimLspMode mode) {
    switch (mode) {
    case Z_NVIM_LSP_INIT_LUA:
        return Z_SV("init.lua");
    case Z_NVIM_LSP_LAZY:
        return Z_SV("lazy");
    case Z_NVIM_LSP_UNKNOWN:
        return Z_SV_NULL;
    }
}

static inline ZapNvimLspMode z_parse_nvim_lsp_mode(ZStringView str) {
    if (z_sv_eql(str, Z_SV("init.lua")) || z_sv_eql(str, Z_SV("init"))) {
        return Z_NVIM_LSP_INIT_LUA;
    } else if (z_sv_eql(str, Z_SV("lazy")) || z_sv_eql(str, Z_SV("lazy.nvim"))) {
        return Z_NVIM_LSP_LAZY;
    } else{
        return Z_NVIM_LSP_UNKNOWN;
    }
}
