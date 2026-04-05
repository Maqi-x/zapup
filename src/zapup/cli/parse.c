#include <zapup/cli/parse.h>
#include <zapup/cli/args.h>

ZCliParseResult z_cli_handle_global_long_flag(ZStringView arg, ZCliArgs* out) {
    return Z_CLI_PARSE_RESULT_OK;
}
ZCliParseResult z_cli_handle_global_short_flag(ZStringView arg, ZCliArgs* out) {
    return Z_CLI_PARSE_RESULT_OK;
}
ZCliParseResult z_cli_handle_global_arg(ZStringView arg, ZCliArgs* out) {
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_long_flag(ZStringView arg, ZCliArgs* out) {
    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        break;
    case Z_CLI_CMD_UNINSTALL:
        break;
    case Z_CLI_CMD_SYNC:
        break;
    case Z_CLI_CMD_HELP:
        break;
    case Z_CLI_CMD_UNKNOWN:
        return z_cli_handle_global_long_flag(arg, out);
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_short_flag(ZStringView arg, ZCliArgs* out) {
    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        break;
    case Z_CLI_CMD_UNINSTALL:
        break;
    case Z_CLI_CMD_SYNC:
        break;
    case Z_CLI_CMD_HELP:
        break;
    case Z_CLI_CMD_UNKNOWN:
        return z_cli_handle_global_short_flag(arg, out);
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_arg(ZStringView arg, ZCliArgs* out) {
    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        break;
    case Z_CLI_CMD_UNINSTALL:
        break;
    case Z_CLI_CMD_SYNC:
        break;
    case Z_CLI_CMD_HELP:
        break;
    case Z_CLI_CMD_UNKNOWN:
        return z_cli_handle_global_arg(arg, out);
    }
    return Z_CLI_PARSE_RESULT_OK;
}

bool z_cli_is_long_flag(ZStringView arg) {
    return z_sv_starts_with(arg, Z_SV("--"));
}
bool z_cli_is_short_flag(ZStringView arg) {
    return z_sv_starts_with(arg, Z_SV("-"));
}

void z_cli_apply_defaults(ZCliArgs* out) {
    out->cmd = Z_CLI_CMD_UNKNOWN;
}

ZCliParseResult z_cli_parse_args(int argc, const char* const* argv, ZCliArgs* out) {
    for (usize i = 1; i < (usize)argc; ++i) {
        ZStringView arg = z_sv_from_cstr(argv[i]);
        if (z_cli_is_long_flag(arg)) {
            ZCliParseResult err = z_cli_handle_long_flag(arg, out);
            if (err.code != Z_CLI_PARSE_OK) {
                return err;
            }
        } else if (z_cli_is_short_flag(arg)) {
            ZCliParseResult err = z_cli_handle_short_flag(arg, out);
            if (err.code != Z_CLI_PARSE_OK) {
                return err;
            }
        } else {
            ZCliParseResult err = z_cli_handle_arg(arg, out);
            if (err.code != Z_CLI_PARSE_OK) {
                return err;
            }
        }
    }
    return Z_CLI_PARSE_RESULT_OK;
}
