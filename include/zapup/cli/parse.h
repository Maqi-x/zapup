#pragma once

#include <zapup/cli/args.h>

typedef enum ZCliParseErrorCode {
    Z_CLI_PARSE_OK,
    Z_CLI_PARSE_UNKNOWN_LONG_FLAG,
    Z_CLI_PARSE_UNKNOWN_SHORT_FLAG,
    Z_CLI_PARSE_UNKNOWN_COMMAND,
    // TODO
} ZCliParseErrorCode;

typedef struct ZCliParseResult {
    ZCliParseErrorCode code;
    union {
        ZStringView str;
        char c;
    } ctx;
} ZCliParseResult;

#define Z_CLI_PARSE_RESULT_OK ((ZCliParseResult) { .code = Z_CLI_PARSE_OK })

ZCliParseResult z_cli_parse_args(int argc, const char* const* argv, ZCliArgs* out);
