#pragma once

#include <zapup/zap/version.h>

typedef enum ZCliCommand {
    Z_CLI_CMD_INSTALL,
    Z_CLI_CMD_UNINSTALL,
    Z_CLI_CMD_TEST,
    Z_CLI_CMD_SYNC,
    Z_CLI_CMD_HELP,
    Z_CLI_CMD_UNKNOWN,
} ZCliCommand;

typedef struct ZCliBuildArgs {
    bool parallel;
    int max_jobs; // 0 means no limit if parallel is true
} ZCliBuildArgs;

typedef struct ZCliInstallArgs {
    ZResolvableZapVersion version;
    ZCliBuildArgs build;
} ZCliInstallArgs;

typedef struct ZCliSyncArgs {
    ZResolvableZapVersion version; // optional
    ZCliBuildArgs build;
} ZCliSyncArgs;

typedef struct ZCliUninstallArgs {
    ZResolvableZapVersion version;
} ZCliUninstallArgs;

typedef struct ZCliTestArgs {
    ZResolvableZapVersion version; // optional
} ZCliTestArgs;

typedef struct ZCliHelpArgs {
    ZCliCommand target;
} ZCliHelpArgs;


typedef struct ZCliGlobalArgs {

} ZCliGlobalArgs;

typedef struct ZCliArgs {
    ZCliGlobalArgs global_args;
    ZCliCommand cmd;
    union {
        ZCliInstallArgs install;
        ZCliUninstallArgs uninstall;
        ZCliTestArgs test;
        ZCliSyncArgs sync;
        ZCliHelpArgs help;
    } cmd_args;
} ZCliArgs;
