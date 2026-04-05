#pragma once

#include <zapup/version.h>

typedef enum ZCliCommand {
    Z_CLI_INSTALL,
    Z_CLI_SYNC,
    Z_CLI_UNINSTALL,
    Z_CLI_HELP,
} ZCliCommand;

typedef struct ZCliInstallArgs {
    ZResolvableZapVersion version;    
} ZCliInstallArgs;

typedef struct ZCliSyncArgs {
    
} ZCliSyncArgs;

typedef struct ZCliUninstallArgs {
    ZResolvableZapVersion version;    
} ZCliUninstallArgs;

typedef struct ZCliHelpArgs {
    
} ZCliHelpArgs;


typedef struct ZCliGlobalArgs {

} ZCliGlobalArgs;

typedef struct ZCliArgs {
    ZCliGlobalArgs global_args;
    ZCliCommand cmd;
    union {
        ZCliInstallArgs install;
        ZCliUninstallArgs uninstall;
        ZCliSyncArgs sync;
        ZCliHelpArgs help;
    } cmd_args;
} ZCliArgs;
