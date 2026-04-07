#pragma once

#include <util/lockfile.h>

#include <zapup/cli/args.h>
#include <zapup/storage.h>
#include <zapup/index.h>

typedef struct ZapupApp {
    ZCliArgs args;
    ZPathsConfig paths;
    ZVersionIndex index;
    ZLockFile indexlock;
} ZapupApp;

void zapup_init(ZapupApp* app);
int zapup_run(ZapupApp* app, int argc, const char* const* argv);
void zapup_destroy(ZapupApp* app);
