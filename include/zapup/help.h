#pragma once

#include <zapup/cli/help/print.h>
#include <zapup/cli/args.h>

void zapup_show_help(const ZHelpInfo* info, ZCliCommand target, FILE* out);
