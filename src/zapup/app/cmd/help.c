#include <zapup/app/app.h>

#include <zapup/cli/help/settings.h>
#include <zapup/cli/help/print.h>

#include <zapup/output.h>

int zapup_exec_help(ZapupApp* app) {
    ZHelpInfo info = zapup_get_help();
    ZCliCommand target = app->args.cmd_args.help.target;

    if (target != Z_CLI_CMD_UNKNOWN) {
        const ZHelpCommand* cmd = zapup_find_help_command(&info, target);
        if (cmd != NULL) {
            z_cli_print_help_cmd(&info, cmd, stdout);
            return 0;
        }
    }

    z_cli_print_help(&info, stdout);
    return 0;
}
