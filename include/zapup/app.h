#pragma once

#include <zapup/cli/args.h>

typedef struct ZapupApp {
    ZCliArgs args;
} ZapupApp;

void zapup_init(ZapupApp* app);
int zapup_run(ZapupApp* app, int argc, const char* const* argv);
void zapup_destroy(ZapupApp* app);
