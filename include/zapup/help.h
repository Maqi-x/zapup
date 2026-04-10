#pragma once

#include <zapup/cli/help/print.h>
#include <zapup/cli/args.h>

void zapup_show_help(const ZHelpInfo* info, ZCliCommand target, FILE* out) {
    if (target != Z_CLI_CMD_UNKNOWN) {
        const ZHelpCommand* cmd = zapup_find_help_command(info, target);
        if (cmd != NULL) {
            z_cli_print_help_cmd(info, cmd, out);
            return;
        }
    }

    z_cli_print_help(info, out);
}
