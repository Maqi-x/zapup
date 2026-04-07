#include <zapup/cli/help/settings.h>

#include <util/arr-len.h>

static ZHelpFlag help_global_flags[] = {
    // TODO
};

static ZHelpCommand help_commands[] = {
    // TODO
};

ZHelpInfo zapup_get_help() {
    return (ZHelpInfo) {
        .name = Z_SV("zapup"),
        .desc = Z_SV("TODO"),

        .global_flags = help_global_flags,
        .global_flags_count = Z_ARRAY_LEN(help_global_flags),

        .cmds = help_commands,
        .command_count = Z_ARRAY_LEN(help_commands),

        .footer = Z_SV("TODO"),
    };
}
