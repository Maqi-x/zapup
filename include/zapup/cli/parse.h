#pragma once

#include <zapup/cli/args.h>
#include <zapup/cli/result.h>

ZCliParseResult z_cli_parse_args(int argc, const char* const* argv, ZCliArgs* out);
