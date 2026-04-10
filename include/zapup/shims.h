#pragma once

#include <util/strbuf.h>
#include <util/pathview.h>

#include <zapup/zap/toolchain.h>

bool z_generate_sh_shim_for(ZPathView self, ZapToolchainElement tool, ZStringBuf* out);
bool z_generate_batch_shim_for(ZPathView self, ZapToolchainElement elem, ZStringBuf* out);
bool z_generate_native_shim_for(ZPathView self, ZapToolchainElement tool, ZStringBuf* out);
