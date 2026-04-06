#pragma once

#include <zapup/cli/args.h>
#include <zapup/storage.h>
#include <zapup/index.h>

typedef struct ZapupApp {
    ZCliArgs args;
    ZPathsConfig paths;
    ZVersionIndex index;
} ZapupApp;

void zapup_init(ZapupApp* app);
int zapup_run(ZapupApp* app, int argc, const char* const* argv);
void zapup_destroy(ZapupApp* app);
