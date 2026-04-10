#pragma once

#include <defs/sv.h>

#include <zapup/output.h>

typedef enum ZCliParseErrorCode {
    Z_CLI_PARSE_OK,
    Z_CLI_PARSE_UNKNOWN_LONG_FLAG,
    Z_CLI_PARSE_UNKNOWN_SHORT_FLAG,
    Z_CLI_PARSE_UNKNOWN_COMMAND,
    Z_CLI_PARSE_UNEXPECTED_ARG,
    Z_CLI_PARSE_EXPECTED_ARG,
    Z_CLI_PARSE_WRONG_ARG_FORMAT,
    Z_CLI_PARSE_COMMAND_EXPECTED,
    Z_CLI_PARSE_MISSING_POSITIONAL_ARG,

    _Z_CLI_PARSE_STOP, // internal
} ZCliParseErrorCode;

typedef struct ZCliParseResult {
    ZCliParseErrorCode code;
    ZStringView arg_name;
    union {
        ZStringView str;
        char c;
    } ctx;
} ZCliParseResult;

#define Z_CLI_PARSE_RESULT_OK ((ZCliParseResult) { .code = Z_CLI_PARSE_OK })
#define Z_CLI_PARSE_RESULT_STOP ((ZCliParseResult) { .code = _Z_CLI_PARSE_STOP })

 void z_parse_result_print(const ZCliParseResult* result, ZOutputType out);
 
