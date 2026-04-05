#include <zapup/cli/parse.h>
#include <zapup/cli/args.h>

#include <util/arr-len.h>

ZCliParseResult z_find_cmd_from_arg(ZStringView arg, ZCliCommand* cmd) {
    static const struct {
        ZStringView name;
        ZCliCommand cmd;
    } table[] = {
        { Z_SV("install"),   Z_CLI_CMD_INSTALL },
        { Z_SV("uninstall"), Z_CLI_CMD_UNINSTALL },
        { Z_SV("sync"),      Z_CLI_CMD_SYNC },
        { Z_SV("help"),      Z_CLI_CMD_HELP },
    };

    for (size_t i = 0; i < Z_ARRAY_LEN(table); i++) {
        if (z_sv_eql(arg, table[i].name)) {
            *cmd = table[i].cmd;
            return Z_CLI_PARSE_RESULT_OK;
        }
    }

    return (ZCliParseResult) {
        .code = Z_CLI_PARSE_UNKNOWN_COMMAND,
        .ctx.str = arg,
    };
}

ZCliParseResult z_cli_handle_global_long_flag(ZStringView flag, ZCliArgs* out) {
    return Z_CLI_PARSE_RESULT_OK;
}
ZCliParseResult z_cli_handle_global_short_flags(ZStringView flags, ZCliArgs* out) {
    return Z_CLI_PARSE_RESULT_OK;
}
ZCliParseResult z_cli_handle_global_arg(ZStringView arg, ZCliArgs* out) {
    ZCliParseResult err = z_find_cmd_from_arg(arg, &out->cmd);
    if (err.code != Z_CLI_PARSE_OK){
        return err;
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_long_flag(ZStringView flag, ZCliArgs* out) {
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
        return z_cli_handle_global_long_flag(flag, out);
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_short_flags(ZStringView flags, ZCliArgs* out) {
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
        return z_cli_handle_global_short_flags(flags, out);
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
            ZStringView flag = z_sv_trim_prefix(arg, Z_SV("--"));
            ZCliParseResult err = z_cli_handle_long_flag(flag, out);
            if (err.code != Z_CLI_PARSE_OK) {
                return err;
            }
        } else if (z_cli_is_short_flag(arg)) {
            ZStringView flags = z_sv_trim_prefix(arg, Z_SV("-"));
            ZCliParseResult err = z_cli_handle_short_flags(flags, out);
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
