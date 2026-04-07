#include <zapup/cli/help/settings.h>

#include <util/arr-len.h>

static ZHelpCommand help[] = {
    // TODO
};

ZHelpInfo zapup_get_help() {
    return (ZHelpInfo) {
        .name = Z_SV("zapup"),
        .desc = Z_SV("TODO"),

        .cmds = help,
        .count = Z_ARRAY_LEN(help),

        .footer = Z_SV("TODO"),
    };
}
