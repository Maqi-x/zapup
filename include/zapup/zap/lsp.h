#pragma once

#include <defs/sv.h>

typedef enum ZapLspTarget {
    Z_LSP_UNKNOWN = 0,
    Z_LSP_NVIM    = 1 << 0,
    Z_LSP_VSC     = 1 << 1,
} ZapLspTarget;

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
