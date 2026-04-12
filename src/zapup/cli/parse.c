#include <zapup/cli/parse.h>
#include <zapup/cli/args.h>

#include <zapup/zap/version.h>
#include <zapup/zap/toolchain.h>

#include <util/arr-len.h>
#include <util/macros.h>

ZCliParseResult z_find_cmd_from_arg(ZStringView arg, ZCliCommand* cmd) {
    static const struct {
        ZStringView name;
        ZCliCommand cmd;
    } table[] = {
        { Z_SV("install"),   Z_CLI_CMD_INSTALL },
        { Z_SV("uninstall"), Z_CLI_CMD_UNINSTALL },
        { Z_SV("reshim"),    Z_CLI_CMD_RESHIM },
        { Z_SV("switch"),    Z_CLI_CMD_SWITCH },
        { Z_SV("select"),    Z_CLI_CMD_SWITCH },
        { Z_SV("which"),     Z_CLI_CMD_WHICH },
        { Z_SV("list"),      Z_CLI_CMD_LIST },
        { Z_SV("show"),      Z_CLI_CMD_SHOW },
        { Z_SV("current"),   Z_CLI_CMD_SHOW },
        { Z_SV("test"),      Z_CLI_CMD_TEST },
        { Z_SV("sync"),      Z_CLI_CMD_SYNC },
        { Z_SV("update"),    Z_CLI_CMD_SYNC },
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

ZCliParseResult z_cli_try_parse_version_into(ZStringView arg, ZapVersion* ver) {
    if (!z_zap_ver_is_null(*ver)) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_UNEXPECTED_ARG,
            .arg_name = Z_SV_NULL,
            .ctx.str = arg,
        };
    }

    ZapVersion result = z_parse_zap_version(arg);
    if (z_zap_ver_is_null(result)) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_WRONG_ARG_FORMAT,
            .arg_name = arg,
            .ctx.str = Z_SV("version"),
        };
    }
    *ver = result;
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_try_parse_tool_into(ZStringView arg, ZapToolchainElement* tool) {
    if (*tool != Z_TOOLCHAIN_ELEMENT_UNKNOWN) {
         return (ZCliParseResult) {
            .code = Z_CLI_PARSE_UNEXPECTED_ARG,
            .arg_name = Z_SV_NULL,
            .ctx.str = arg,
        };
    }

    ZapToolchainElement parsed = z_parse_zap_toolchain_element(arg);
    if (parsed == Z_TOOLCHAIN_ELEMENT_UNKNOWN) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_WRONG_ARG_FORMAT,
            .arg_name = Z_SV("tool"),
            .ctx.str = arg,
        };
    }
    *tool = parsed;
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_parse_jobs(ZStringView val, int* jobs) {
    if (val.len > 0 && val.data[0] == '=') {
        val = z_sv_slice(val, 1, val.len);
    }

    if (val.len == 0) {
        *jobs = 0;
        return Z_CLI_PARSE_RESULT_OK;
    }

    char buf[32];
    if (val.len >= sizeof(buf)) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_WRONG_ARG_FORMAT,
            .arg_name = Z_SV("max-jobs"),
            .ctx.str = val,
        };
    }
    memcpy(buf, val.data, val.len);
    buf[val.len] = '\0';

    char* end;
    long res = strtol(buf, &end, 10);
    if (*end != '\0' || res < 0) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_WRONG_ARG_FORMAT,
            .arg_name = Z_SV("max-jobs"),
            .ctx.str = val,
        };
    }
    *jobs = (int)res;
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_set_bool_flag_long(ZStringView name, bool* out) {
    if (*out) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_LONG_FLAG_REDECLARED,
            .arg_name = name,
            .ctx.str = name,
        };
    }

    *out = true;
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_set_bool_flag_short(ZStringView name, char c, bool* out) {
    if (*out) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_SHORT_FLAG_REDECLARED,
            .arg_name = name,
            .ctx.c = c,
        };
    }

    *out = true;
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
        return Z_CLI_PARSE_RESULT_STOP;
    } else if (z_sv_eql(flag, Z_SV("global")) || z_sv_eql(flag, Z_SV("ignore-local"))) {
        return z_cli_set_bool_flag_long(flag, &out->global_args.ignore_local);
    }
    return z_cli_unknown_long_flag(flag);
}

ZCliParseResult z_cli_handle_global_short_flag(char flag, ZCliArgs* out) {
    if (flag == 'h') {
        out->cmd_args.help.target = out->cmd;
        out->cmd = Z_CLI_CMD_HELP;
        return Z_CLI_PARSE_RESULT_STOP;
    } else if (flag == 'g') {
        return z_cli_set_bool_flag_short(Z_SV("global"), flag, &out->global_args.ignore_local);
    }
    return z_cli_unknown_short_flag(flag);
}

ZCliParseResult z_cli_handle_global_arg(ZStringView arg, ZCliArgs* out) {
    return z_find_cmd_from_arg(arg, &out->cmd);
}

static ZCliBuildArgs* z_cli_get_build_args(ZCliArgs* out) {
    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL: return &out->cmd_args.install.build;
    case Z_CLI_CMD_SYNC:    return &out->cmd_args.sync.build;
    default:                return NULL;
    }
}

ZCliParseResult z_cli_handle_cmd_long_flag(ZStringView flag, ZCliArgs* out) {
    ZCliBuildArgs* build = z_cli_get_build_args(out);
    if (build) {
        if (z_sv_eql(flag, Z_SV("parallel"))) {
            build->parallel = true;
            build->max_jobs = 0;
            return Z_CLI_PARSE_RESULT_OK;
        }
        if (z_sv_starts_with(flag, Z_SV("parallel="))) {
            build->parallel = true;
            ZStringView val = z_sv_trim_prefix(flag, Z_SV("parallel="));
            return z_cli_parse_jobs(val, &build->max_jobs);
        }
        if (z_sv_eql(flag, Z_SV("test")) || z_sv_eql(flag, Z_SV("run-tests"))) {
            return z_cli_set_bool_flag_long(Z_SV("test"), &build->run_tests);
        }
    }

    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        if (z_sv_eql(flag, Z_SV("switch")) || z_sv_eql(flag, Z_SV("select"))) {
            return z_cli_set_bool_flag_long(Z_SV("switch"), &out->cmd_args.install.switch_);
        } else {
            return z_cli_unknown_long_flag(flag);
        }
        break;

    case Z_CLI_CMD_SWITCH:
        if (z_sv_eql(flag, Z_SV("local")))  {
            return z_cli_set_bool_flag_long(Z_SV("local"), &out->cmd_args.switch_.local);
        } else {
            return z_cli_unknown_long_flag(flag);
        }
        break;

    case Z_CLI_CMD_UNINSTALL:
    case Z_CLI_CMD_RESHIM:
    case Z_CLI_CMD_WHICH:
    case Z_CLI_CMD_LIST:
    case Z_CLI_CMD_SHOW:
    case Z_CLI_CMD_TEST:
    case Z_CLI_CMD_SYNC:
    case Z_CLI_CMD_HELP:
        return z_cli_unknown_long_flag(flag);
    case Z_CLI_CMD_UNKNOWN:
        Z_UNREACHABLE("z_cli_handle_cmd_long_flag() should not be called with Z_CLI_CMD_UNKNOWN");
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_handle_cmd_short_flag(ZStringView flags, usize* i, ZCliArgs* out) {
    char flag = flags.data[*i];
    ZCliBuildArgs* build = z_cli_get_build_args(out);
    if (build) {
        if (flag == 'j') {
            build->parallel = true;
            ZStringView val = z_sv_slice(flags, *i + 1, flags.len);
            ZCliParseResult res = z_cli_parse_jobs(val, &build->max_jobs);
            if (res.code == Z_CLI_PARSE_OK) {
                *i = flags.len;
            }
            return res;
        } else if (flag == 't') {
            return z_cli_set_bool_flag_short(Z_SV("test"), 't', &build->run_tests);
        }
    }

    switch (out->cmd) {
    case Z_CLI_CMD_INSTALL:
        if (flag == 's') {
            return z_cli_set_bool_flag_short(Z_SV("switch"), 's', &out->cmd_args.install.switch_);
        } else {
            return z_cli_unknown_short_flag(flag);
        }
        break;

    case Z_CLI_CMD_SWITCH:
        if (flag == 'l') {
            return z_cli_set_bool_flag_short(Z_SV("local"), flag, &out->cmd_args.switch_.local);
        } else {
            return z_cli_unknown_short_flag(flag);
        }
        break;

    case Z_CLI_CMD_UNINSTALL:
    case Z_CLI_CMD_RESHIM:
    case Z_CLI_CMD_WHICH:
    case Z_CLI_CMD_LIST:
    case Z_CLI_CMD_SHOW:
    case Z_CLI_CMD_TEST:
    case Z_CLI_CMD_SYNC:
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
    case Z_CLI_CMD_RESHIM:
        return z_cli_try_parse_tool_into(arg, &out->cmd_args.reshim.tool);
    case Z_CLI_CMD_SWITCH:
        return z_cli_try_parse_version_into(arg, &out->cmd_args.switch_.version);
    case Z_CLI_CMD_TEST:
        return z_cli_try_parse_version_into(arg, &out->cmd_args.test.version);
    case Z_CLI_CMD_SYNC:
        return z_cli_try_parse_version_into(arg, &out->cmd_args.sync.version);
    case Z_CLI_CMD_WHICH:
        // hotfix: ugly but works (i think)
        if (z_zap_ver_is_null(out->cmd_args.which.version) && out->cmd_args.which.tool == Z_TOOLCHAIN_ELEMENT_UNKNOWN) {
            ZCliParseResult tool_res = z_cli_try_parse_tool_into(arg, &out->cmd_args.which.tool);
            if (tool_res.code == Z_CLI_PARSE_OK) {
                return Z_CLI_PARSE_RESULT_OK;
            }
            if (tool_res.code != Z_CLI_PARSE_WRONG_ARG_FORMAT) {
                return tool_res;
            }
            return z_cli_try_parse_version_into(arg, &out->cmd_args.which.version);
        }

        if (!z_zap_ver_is_null(out->cmd_args.which.version) && out->cmd_args.which.tool == Z_TOOLCHAIN_ELEMENT_UNKNOWN) {
            return z_cli_try_parse_tool_into(arg, &out->cmd_args.which.tool);
        }

        return z_cli_unexpected_arg(arg);
    case Z_CLI_CMD_LIST:
    case Z_CLI_CMD_SHOW:
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

ZCliParseResult z_cli_handle_short_flag(ZStringView flags, usize* i, ZCliArgs* out) {
    char flag = flags.data[*i];
    if (out->cmd == Z_CLI_CMD_UNKNOWN) {
        return z_cli_handle_global_short_flag(flag, out);
    }
    ZCliParseResult res = z_cli_handle_cmd_short_flag(flags, i, out);
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
    out->global_args.ignore_local = false;
}

void z_cli_apply_build_defaults(ZCliBuildArgs* build) {
    build->parallel = false;
    build->max_jobs = 0;
    build->run_tests = false;
}

void z_cli_apply_command_defaults(ZCliCommand cmd, ZCliArgs* out) {
    ZCliBuildArgs* build = z_cli_get_build_args(out);
    if (build) {
        z_cli_apply_build_defaults(build);
    }

    switch (cmd) {
    case Z_CLI_CMD_INSTALL:
        out->cmd_args.install.version = Z_ZAP_VERSION_NULL;
        break;
    case Z_CLI_CMD_UNINSTALL:
        out->cmd_args.uninstall.version = Z_ZAP_VERSION_NULL;
        break;
    case Z_CLI_CMD_RESHIM:
        out->cmd_args.reshim.tool = Z_TOOLCHAIN_ELEMENT_UNKNOWN;
        break;
    case Z_CLI_CMD_SWITCH:
        out->cmd_args.switch_.version = Z_ZAP_VERSION_NULL;
        out->cmd_args.switch_.local = false;
        break;
    case Z_CLI_CMD_LIST:
    case Z_CLI_CMD_SHOW:
        break;
    case Z_CLI_CMD_TEST:
        out->cmd_args.test.version = Z_ZAP_VERSION_NULL;
        break;
    case Z_CLI_CMD_SYNC:
        out->cmd_args.sync.version = Z_ZAP_VERSION_NULL;
        break;
    case Z_CLI_CMD_WHICH:
        out->cmd_args.which.version = Z_ZAP_VERSION_NULL;
        out->cmd_args.which.tool = Z_TOOLCHAIN_ELEMENT_UNKNOWN;
        break;
    case Z_CLI_CMD_HELP:
       out->cmd_args.help.target = Z_CLI_CMD_UNKNOWN;
       break;
    case Z_CLI_CMD_UNKNOWN:
        Z_UNREACHABLE("z_cli_apply_command_defaults() should not be called with Z_CLI_CMD_UNKNOWN parameter");
    }
}

ZCliParseResult z_cli_check_version(ZapVersion ver) {
    if (z_zap_ver_is_null(ver)) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_MISSING_POSITIONAL_ARG,
            .arg_name = Z_SV("version"),
        };
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_check_tool(ZapToolchainElement tool) {
    if (tool == Z_TOOLCHAIN_ELEMENT_UNKNOWN) {
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_MISSING_POSITIONAL_ARG,
            .arg_name = Z_SV("tool"),
        };
    }
    return Z_CLI_PARSE_RESULT_OK;
}

ZCliParseResult z_cli_validate_args(ZCliArgs* args) {
    switch (args->cmd) {
    case Z_CLI_CMD_UNKNOWN:
        return (ZCliParseResult) {
            .code = Z_CLI_PARSE_COMMAND_EXPECTED,
        };
    case Z_CLI_CMD_INSTALL:
        return z_cli_check_version(args->cmd_args.install.version);
    case Z_CLI_CMD_UNINSTALL:
        return z_cli_check_version(args->cmd_args.uninstall.version);
    case Z_CLI_CMD_RESHIM:
        return z_cli_check_tool(args->cmd_args.which.tool);
    case Z_CLI_CMD_SWITCH:
        return z_cli_check_version(args->cmd_args.switch_.version);
    case Z_CLI_CMD_WHICH:
        return z_cli_check_tool(args->cmd_args.which.tool);

    case Z_CLI_CMD_LIST:
    case Z_CLI_CMD_SHOW:
    case Z_CLI_CMD_TEST:
    case Z_CLI_CMD_SYNC:
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

ZCliParseResult z_cli_process_token(ZStringView arg, bool* stop_parsing_flags, ZCliArgs* out) {
    if (*stop_parsing_flags) {
        return z_cli_handle_arg(arg, out);
    }

    if (z_sv_eql(arg, Z_SV("--"))) {
        *stop_parsing_flags = true;
        return Z_CLI_PARSE_RESULT_OK;
    }

    if (z_cli_is_long_flag(arg)) {
        ZStringView flag = z_sv_trim_prefix(arg, Z_SV("--"));
        return z_cli_handle_long_flag(flag, out);
    }

    if (z_cli_is_short_flag(arg)) {
        ZStringView flags = z_sv_trim_prefix(arg, Z_SV("-"));
        for (usize j = 0; j < flags.len; ++j) {
            ZCliParseResult err = z_cli_handle_short_flag(flags, &j, out);
            Z_CLI_HANDLE_ERR(err);
        }
        return Z_CLI_PARSE_RESULT_OK;
    }

    return z_cli_handle_arg(arg, out);
}

ZCliParseResult z_cli_parse_args(int argc, const char* const* argv, ZCliArgs* out) {
    z_cli_apply_defaults(out);
    bool stop_parsing_flags = false;
    for (usize i = 1; i < (usize) argc; ++i) {
        ZCliCommand cmd_old = out->cmd;
        ZStringView arg = z_sv_from_cstr(argv[i]);

        ZCliParseResult err = z_cli_process_token(arg, &stop_parsing_flags, out);

        Z_CLI_HANDLE_ERR(err);        if (out->cmd != cmd_old) {
            z_cli_apply_command_defaults(out->cmd, out);
        }
    }
    Z_CLI_HANDLE_ERR(z_cli_validate_args(out));
    return Z_CLI_PARSE_RESULT_OK;
}
