#pragma once

#include <zapup/cli/args.h>

typedef enum ZCliParseErrorCode {
    Z_CLI_PARSE_UNKNOWN_FLAG,
    Z_CLI_PARSE_UNKNOWN_COMMAND,
    // TODO
} ZCliParseErrorCode;

typedef struct ZCliParseError {
    ZCliParseErrorCode code;
    union {
        // TODO
    };
} ZCliParseError;

ZCliParseError z_cli_parse_args(int argc, const char* const* argv, ZCliArgs* out);
