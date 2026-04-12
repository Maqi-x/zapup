#include <zapup/cli/help/settings.h>
#include <zapup/cli/ansi.h>

#include <util/arr-len.h>

static ZHelpFlag help_global_flags[] = {
    { Z_SV("--help, -h"),   Z_SV("Show this help message") },
    { Z_SV("--global, -g"), Z_SV("Ignore local project config and use only global configuration") },
};

static ZHelpFlag help_build_flags[] = {
    { Z_SV("--parallel[=N], -j[N]"),   Z_SV("Enable parallel building with N jobs") },
    { Z_SV("--test, --run-tests, -t"), Z_SV("Run tests after instalation") },
};

static ZHelpCommand help_commands[] = {
    {
        .cmd = Z_CLI_CMD_INSTALL,
        .name = Z_SV("install"),
        .desc = Z_SV("Install a specific Zap version"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("version", "Zap version to install", false)
        ),
        .flags = Z_HELP_FLAG_ARRAY(
            { Z_SV("--switch, --select, -s"), Z_SV("Switch to newly installed version") },
        ),
    },
    {
        .cmd = Z_CLI_CMD_UNINSTALL,
        .name = Z_SV("uninstall"),
        .desc = Z_SV("Uninstall a specific Zap version"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("version", "Installed Zap version to remove", false)
        ),
        .flags = Z_HELP_NO_FLAGS,
    },
    {
        .cmd = Z_CLI_CMD_RESHIM,
        .name = Z_SV("reshim"),
        .desc = Z_SV("Recreate shims for a specific Zap tool"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("tool", "Toolchain element to re-shim (zapc / zap-lsp); All if not specified", true)
        ),
        .flags = Z_HELP_NO_FLAGS,
    },
    {
        .cmd = Z_CLI_CMD_SWITCH,
        .name = Z_SV("switch"),
        .desc = Z_SV("Switch the current Zap version"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("version", "Installed Zap version to set as current", false)
        ),
        .flags = Z_HELP_FLAG_ARRAY(
            { Z_SV("--local, -l"), Z_SV("Change version in local config instead of global") },
        ),
    },
    {
        .cmd = Z_CLI_CMD_LIST,
        .name = Z_SV("list"),
        .desc = Z_SV("List available and installed Zap versions"),
        .usage = Z_HELP_NO_USAGE_FIELDS,
        .flags = Z_HELP_NO_FLAGS,
    },
    {
        .cmd = Z_CLI_CMD_SHOW,
        .name = Z_SV("show"),
        .desc = Z_SV("Show currently active version"),
        .usage = Z_HELP_NO_USAGE_FIELDS,
        .flags = Z_HELP_NO_FLAGS,
    },
    {
        .cmd = Z_CLI_CMD_WHICH,
        .name = Z_SV("which"),
        .desc = Z_SV("Show the path to a Zap tool"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("version", "Optional Zap version to inspect", true),
            Z_HELP_USAGE("tool", "Toolchain element to locate (zapc / zap-lsp)", false),
        ),
        .flags = Z_HELP_NO_FLAGS,
    },
    {
        .cmd = Z_CLI_CMD_TEST,
        .name = Z_SV("test"),
        .desc = Z_SV("Test a specific Zap version or all versions"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("version", "Optional Zap version to test; All if not specified", true)
        ),
        .flags = Z_HELP_NO_FLAGS,
    },
    {
        .cmd = Z_CLI_CMD_SYNC,
        .name = Z_SV("sync"),
        .desc = Z_SV("Sync Zap versions with the remote"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("version", "Optional specific version to sync; All if not specified", true)
        ),
        .flags = Z_HELP_NO_FLAGS,
    },
    {
        .cmd = Z_CLI_CMD_HELP,
        .name = Z_SV("help"),
        .desc = Z_SV("Show general help or details for one command"),
        .usage = Z_HELP_USAGE_FIELDS(
            Z_HELP_USAGE("command", "Optional command name to show detailed help for", true)
        ),
        .flags = Z_HELP_NO_FLAGS,
    },
};

ZHelpInfo zapup_get_help() {
    return (ZHelpInfo) {
        .name = Z_SV("zapup"),
        .desc = Z_SV(
            " Version manager for The " BOLD "Zap Programming Language: " RESET "\n"
            " " BOLD "github: " RESET " https://github.com/thezaplang/zap\n"
            " " BOLD "website:" RESET " https://zaplang.xyz"
        ),

        .global_flags = Z_HELP_FLAG_ARRAY_FROM(help_global_flags),
        .build_flags = Z_HELP_FLAG_ARRAY_FROM(help_build_flags),

        .cmds = help_commands,
        .command_count = Z_ARRAY_LEN(help_commands),

        .footer = Z_SV(
            "For bug reports and more information visit: https://github.com/maqi-x/zapup.\n"
            "Licensed under the GNU General Public License v3."
        ),
    };
}

const ZHelpCommand* zapup_find_help_command(const ZHelpInfo* info, ZCliCommand cmd) {
    if (info == NULL || info->cmds == NULL) {
        return NULL;
    }

    for (const ZHelpCommand* it = info->cmds; it < info->cmds + info->command_count; ++it) {
        if (it->cmd == cmd) {
            return it;
        }
    }

    return NULL;
}
