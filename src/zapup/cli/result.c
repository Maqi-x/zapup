#include <zapup/cli/result.h>

void z_parse_result_print(const ZCliParseResult* result, ZOutputType out) {
    switch (result->code) {
    case Z_CLI_PARSE_COMMAND_EXPECTED:
        return z_output(out, "Command expected");
    case Z_CLI_PARSE_UNKNOWN_COMMAND:
        return z_output(out, "Unknown command: " Z_SV_FMT, Z_SV_FARG(result->ctx.str));
    
    case Z_CLI_PARSE_UNKNOWN_LONG_FLAG:
        return z_output(out, "Unknown flag: --" Z_SV_FMT, Z_SV_FARG(result->ctx.str));
    case Z_CLI_PARSE_UNKNOWN_SHORT_FLAG:
        return z_output(out, "Unknown flag: -%c", result->ctx.c);
    case Z_CLI_PARSE_UNEXPECTED_ARG:
        return z_output(out, "Unexpected argument: " Z_SV_FMT, Z_SV_FARG(result->ctx.str));

    case Z_CLI_PARSE_WRONG_ARG_FORMAT:
        return z_output(out, "Wrong format of argument " Z_SV_FMT, Z_SV_FARG(result->arg_name));
    case Z_CLI_PARSE_MISSING_POSITIONAL_ARG:
        return z_output(out, "Missing positional argument " Z_SV_FMT, Z_SV_FARG(result->arg_name));
    case Z_CLI_PARSE_EXPECTED_ARG:
        return z_output(out, "Expected argument " Z_SV_FMT, Z_SV_FARG(result->arg_name));

    case Z_CLI_PARSE_OK:
    case _Z_CLI_PARSE_STOP:
        return;
    }
}
