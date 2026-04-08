#pragma once

#include <defs/sv.h>

typedef enum ZapToolchainElement {
    Z_TOOLCHAIN_ELEMENT_ZAPC,
    Z_TOOLCHAIN_ELEMENT_ZAP_LSP,
    Z_TOOLCHAIN_ELEMENT_UNKNOWN,
} ZapToolchainElement;

static inline ZStringView z_format_zap_toolchain_element(ZapToolchainElement tool) {
    switch (tool) {
    case Z_TOOLCHAIN_ELEMENT_ZAPC:
        return Z_SV("zapc");
    case Z_TOOLCHAIN_ELEMENT_ZAP_LSP:
        return Z_SV("zap-lsp");
    case Z_TOOLCHAIN_ELEMENT_UNKNOWN:
        return Z_SV_NULL;
    }
}

static inline ZapToolchainElement z_parse_zap_toolchain_element(ZStringView str) {
    if (z_sv_eql(str, Z_SV("zapc"))) {
        return Z_TOOLCHAIN_ELEMENT_ZAPC;
    } else if (z_sv_eql(str, Z_SV("zap-lsp")) || z_sv_eql(str, Z_SV("zapd"))) {
        return Z_TOOLCHAIN_ELEMENT_ZAP_LSP;
    } else{
        return Z_TOOLCHAIN_ELEMENT_UNKNOWN;
    }
}
