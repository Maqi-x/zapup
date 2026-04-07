#include <zapup/app/app.h>

#include <zapup/cli/help/settings.h>
#include <zapup/cli/help/print.h>

#include <zapup/output.h>

int zapup_exec_help(ZapupApp* app) {
    (void) app;
    ZHelpInfo info = zapup_get_help();
    z_cli_print_help(&info, stdout);
    return 0;
}
