#pragma once

#include <zapup/cli/args.h>

typedef enum ZCliParseErrorCode {
    Z_CLI_PARSE_OK,
    Z_CLI_PARSE_UNKNOWN_FLAG,
    Z_CLI_PARSE_UNKNOWN_COMMAND,
    // TODO
} ZCliParseErrorCode;

typedef struct ZCliParseError {
    ZCliParseErrorCode code;
    union {
        ZStringView str;
    } ctx;
} ZCliParseError;

#define Z_CLI_PARSE_MK_OK() ((ZCliParseError) { .code = Z_CLI_PARSE_OK })

ZCliParseError z_cli_parse_args(int argc, const char* const* argv, ZCliArgs* out);
