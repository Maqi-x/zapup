#include <zapup/cli/help/settings.h>

#include <util/arr-len.h>

static ZHelpCommand help[] = {
    // TODO
};

ZHelpInfo zapup_get_help() {
    return (ZHelpInfo) {
        .cmds = help,
        .count = Z_ARRAY_LEN(help),
    };
}
