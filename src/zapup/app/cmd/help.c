#include <zapup/app/app.h>

#include <zapup/help.h>
#include <zapup/output.h>

int zapup_exec_help(ZapupApp* app) {
    ZHelpInfo info = zapup_get_help();
    ZCliCommand target = app->args.cmd_args.help.target;
    zapup_show_help(&info, target, stdout);
    return 0;
}
