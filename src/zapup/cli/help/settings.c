#include <zapup/cli/help/settings.h>

#include <util/arr-len.h>

static ZHelpFlag help_global_flags[] = {
    { Z_SV("-h, --help"), Z_SV("Show this help message") },
};

static ZHelpFlag help_build_flags[] = {
    { Z_SV("-j[N], --parallel[=N]"), Z_SV("Enable parallel building with N jobs") },
};

static ZHelpCommand help_commands[] = {
    { Z_SV("install"),   Z_SV("Install a specific zap version"),   NULL },
    { Z_SV("uninstall"), Z_SV("Uninstall a specific zap version"), NULL },
    { Z_SV("sync"),      Z_SV("Sync project with a zap version"),  NULL },
    { Z_SV("help"),      Z_SV("Show help for a specific command"), NULL },
};

ZHelpInfo zapup_get_help() {
    return (ZHelpInfo) {
        .name = Z_SV("zapup"),
        .desc = Z_SV(
            "Version manager for The Zap Programming Language:\n"
            "github: https://github.com/thezaplang/zap\n"
            "website: https://zaplang.xyz"
        ),

        .global_flags = help_global_flags,
        .global_flags_count = Z_ARRAY_LEN(help_global_flags),

        .build_flags = help_build_flags,
        .build_flags_count = Z_ARRAY_LEN(help_build_flags),

        .cmds = help_commands,
        .command_count = Z_ARRAY_LEN(help_commands),

        .footer = Z_SV(
            "For bug reports and more information visit: https://github.com/maqi-x/zapup.\n"
            " Licensed under the GNU General Public License v3."
        ),
    };
}
