#include <zapup/cli/parse.h>
#include <zapup/cli/args.h>

#include <zapup/version.h>

#include <util/arr-len.h>
#include <util/macros.h>

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

ZCliParseResult z_cli_try_parse_version_into(ZStringView arg, ZResolvableZapVersion* ver) {
    if (!z_zap_ver_is_null(*ver)) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_UNEXPECTED_ARG,
            .ctx.str = arg,
        };
    }

    ZResolvableZapVersion result = z_parse_zap_version(arg);
    if (z_zap_ver_is_null(result)) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_WRONG_ARG_FORMAT,
            .ctx.str = arg,
        };
    }
    *ver = result;
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_unexpected_arg(ZStringView arg) {
    return (ZCliParseResult) {
        .code = Z_CLI_PARSE_UNEXPECTED_ARG,
        .ctx.str = arg,
    };
}
ZCliParseResult z_cli_unknown_long_flag(ZStringView flag) {
    return (ZCliParseResult) {
        .code = Z_CLI_PARSE_UNKNOWN_LONG_FLAG,
        .ctx.str = flag,
    };
}
ZCliParseResult z_cli_unknown_short_flag(char flag) {
    return (ZCliParseResult) {
        .code = Z_CLI_PARSE_UNKNOWN_SHORT_FLAG,
        .ctx.c = flag,
    };
}

ZCliParseResult z_cli_handle_global_long_flag(ZStringView flag, ZCliArgs* out) {
    if (z_sv_eql(flag, Z_SV("help"))) {
        out->cmd_args.help.target = out->cmd;
        out->cmd = Z_CLI_CMD_HELP;
        return (ZCliParseResult) { .code = _Z_CLI_PARSE_STOP };
    }
    return z_cli_unknown_long_flag(flag);
}

ZCliParseResult z_cli_handle_global_short_flag(char flag, ZCliArgs* out) {
    if (flag == 'h') {
        out->cmd_args.help.target = out->cmd;
        out->cmd = Z_CLI_CMD_HELP;
        return (ZCliParseResult) { .code = _Z_CLI_PARSE_STOP };
    }
    return z_cli_unknown_short_flag(flag);
}

ZCliParseResult z_cli_handle_global_arg(ZStringView arg, ZCliArgs* out) {
    return z_find_cmd_from_arg(arg, &out->cmd);
}

ZCliParseResult z_cli_handle_cmd_long_flag(ZStringView flag, ZCliArgs* out) {
    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        return z_cli_unknown_long_flag(flag);
    case Z_CLI_CMD_UNINSTALL:
        return z_cli_unknown_long_flag(flag);
    case Z_CLI_CMD_SYNC:
        return z_cli_unknown_long_flag(flag);
    case Z_CLI_CMD_HELP:
        return z_cli_unknown_long_flag(flag);
    case Z_CLI_CMD_UNKNOWN:
        Z_UNREACHABLE("z_cli_handle_cmd_long_flag() should not be called with Z_CLI_CMD_UNKNOWN");
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_cmd_short_flag(char flag, ZCliArgs* out) {
    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        return z_cli_unknown_short_flag(flag);
    case Z_CLI_CMD_UNINSTALL:
        return z_cli_unknown_short_flag(flag);
    case Z_CLI_CMD_SYNC:
        return z_cli_unknown_short_flag(flag);
    case Z_CLI_CMD_HELP:
        return z_cli_unknown_short_flag(flag);
    case Z_CLI_CMD_UNKNOWN:
        Z_UNREACHABLE("z_cli_handle_cmd_short_flag() should not be called with Z_CLI_CMD_UNKNOWN");
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_cmd_arg(ZStringView arg, ZCliArgs* out) {
    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        return z_cli_try_parse_version_into(arg, &out->cmd_args.install.version);
    case Z_CLI_CMD_UNINSTALL:
        return z_cli_try_parse_version_into(arg, &out->cmd_args.uninstall.version);
    case Z_CLI_CMD_SYNC:
        return z_cli_unexpected_arg(arg);
    case Z_CLI_CMD_HELP:
        if (out->cmd_args.help.target != Z_CLI_CMD_UNKNOWN) {
            return z_cli_unexpected_arg(arg);
        }
        return z_find_cmd_from_arg(arg, &out->cmd_args.help.target);
    case Z_CLI_CMD_UNKNOWN:
        Z_UNREACHABLE("z_cli_handle_cmd_arg() should not be called with Z_CLI_CMD_UNKNOWN");
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_long_flag(ZStringView flag, ZCliArgs* out) {
    if (out->cmd == Z_CLI_CMD_UNKNOWN) {
        return z_cli_handle_global_long_flag(flag, out);
    }
    ZCliParseResult res = z_cli_handle_cmd_long_flag(flag, out);
    if (res.code == Z_CLI_PARSE_UNKNOWN_LONG_FLAG) {
        return z_cli_handle_global_long_flag(flag, out);
    }
    return res;
}

ZCliParseResult z_cli_handle_short_flag(char flag, ZCliArgs* out) {
    if (out->cmd == Z_CLI_CMD_UNKNOWN) {
        return z_cli_handle_global_short_flag(flag, out);
    }
    ZCliParseResult res = z_cli_handle_cmd_short_flag(flag, out);
    if (res.code == Z_CLI_PARSE_UNKNOWN_SHORT_FLAG) {
        return z_cli_handle_global_short_flag(flag, out);
    }
    return res;
}

ZCliParseResult z_cli_handle_arg(ZStringView arg, ZCliArgs* out) {
    if (out->cmd == Z_CLI_CMD_UNKNOWN) {
        return z_cli_handle_global_arg(arg, out);
    }
    return z_cli_handle_cmd_arg(arg, out);
}

bool z_cli_is_long_flag(ZStringView arg) {
    return z_sv_starts_with(arg, Z_SV("--"));
}
bool z_cli_is_short_flag(ZStringView arg) {
    return z_sv_starts_with(arg, Z_SV("-")) && arg.len > 1;
}

void z_cli_apply_defaults(ZCliArgs* out) {
    out->cmd = Z_CLI_CMD_UNKNOWN;
}
void z_cli_apply_command_defaults(ZCliCommand cmd, ZCliArgs* out) {
    switch (cmd) {
    case Z_CLI_CMD_INSTALL:
        out->cmd_args.install.version = Z_ZAP_VERSION_NULL;
        break;
    case Z_CLI_CMD_UNINSTALL:
        out->cmd_args.uninstall.version = Z_ZAP_VERSION_NULL;
        break;
    case Z_CLI_CMD_SYNC:
        break;
    case Z_CLI_CMD_HELP:
       out->cmd_args.help.target = Z_CLI_CMD_UNKNOWN; 
       break;
    case Z_CLI_CMD_UNKNOWN:
        Z_UNREACHABLE("z_cli_apply_command_defaults() should not be called with Z_CLI_CMD_UNKNOWN parameter");
    }
}

ZCliParseResult z_cli_validate_args(ZCliArgs* args) {
    switch (args->cmd) {
    case Z_CLI_CMD_UNKNOWN:
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_COMMAND_EXPECTED,
        };
    case Z_CLI_CMD_INSTALL:
        if (z_zap_ver_is_null(args->cmd_args.install.version)) {
            return (ZCliParseResult) {
                .code = Z_CLI_PARSE_MISSING_POSITIONAL_ARG,
                .ctx.str = Z_SV("version"),
            };
        }
        break;
    case Z_CLI_CMD_UNINSTALL:
        if (z_zap_ver_is_null(args->cmd_args.uninstall.version)) {
            return (ZCliParseResult) {
                .code = Z_CLI_PARSE_MISSING_POSITIONAL_ARG,
                .ctx.str = Z_SV("version"),
            };
        }
        break;
    case Z_CLI_CMD_SYNC:
        break;
    case Z_CLI_CMD_HELP:
        break;
    }
    return Z_CLI_PARSE_RESULT_OK;
}

#define Z_CLI_HANDLE_ERR(ERR)               \
    if ((ERR).code == _Z_CLI_PARSE_STOP) {  \
        return Z_CLI_PARSE_RESULT_OK;       \
    }                                       \
    if ((ERR).code != Z_CLI_PARSE_OK) {     \
        return (ERR);                       \
    }

ZCliParseResult z_cli_parse_args(int argc, const char* const* argv, ZCliArgs* out) {
    z_cli_apply_defaults(out);
    for (usize i = 1; i < (usize)argc; ++i) {
        ZCliCommand cmd_old = out->cmd;
        ZStringView arg = z_sv_from_cstr(argv[i]);
        if (z_cli_is_long_flag(arg)) {
            ZStringView flag = z_sv_trim_prefix(arg, Z_SV("--"));
            ZCliParseResult err = z_cli_handle_long_flag(flag, out);
            Z_CLI_HANDLE_ERR(err);
        } else if (z_cli_is_short_flag(arg)) {
            ZStringView flags = z_sv_trim_prefix(arg, Z_SV("-"));
            for (usize i = 0; i < flags.len; ++i) {
                ZCliParseResult err = z_cli_handle_short_flag(flags.data[i], out);
                Z_CLI_HANDLE_ERR(err);
            }
        } else {
            ZCliParseResult err = z_cli_handle_arg(arg, out);
            Z_CLI_HANDLE_ERR(err);
        }

        if (out->cmd != cmd_old) {
            z_cli_apply_command_defaults(out->cmd, out);
        }
    }
    Z_CLI_HANDLE_ERR(z_cli_validate_args(out));
    return Z_CLI_PARSE_RESULT_OK;
}
