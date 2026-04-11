#include <zapup/app/app.h>
#include <zapup/app/switch.h>

int zapup_exec_switch(ZapupApp* app) {
    const ZCliSwitchArgs* args = &app->args.cmd_args.switch_;
    return zapup_switch_to_version(app, args->version);
}
