#pragma once

#include <zapup/zap/toolchain.h>
#include <zapup/zap/version.h>

typedef enum ZCliCommand {
    Z_CLI_CMD_INSTALL,
    Z_CLI_CMD_UNINSTALL,
    Z_CLI_CMD_RESHIM,
    Z_CLI_CMD_SWITCH,
    Z_CLI_CMD_WHICH,
    Z_CLI_CMD_LIST,
    Z_CLI_CMD_SHOW,
    Z_CLI_CMD_TEST,
    Z_CLI_CMD_SYNC,
    Z_CLI_CMD_HELP,
    Z_CLI_CMD_UNKNOWN,
} ZCliCommand;

typedef struct ZCliBuildArgs {
    bool parallel;
    int max_jobs; // 0 means no limit if parallel is true
    bool run_tests;
} ZCliBuildArgs;

typedef struct ZCliInstallArgs {
    ZapVersion version;
    ZCliBuildArgs build;
    bool switch_;
} ZCliInstallArgs;

typedef struct ZCliUninstallArgs {
    ZapVersion version;
} ZCliUninstallArgs;

typedef struct ZCliReshimArgs {
    ZapToolchainElement tool; // optional
} ZCliReshimArgs;

typedef struct ZCliSyncArgs {
    ZapVersion version; // optional
    ZCliBuildArgs build;
} ZCliSyncArgs;

typedef struct ZCliListArgs {
} ZCliListArgs;

typedef struct ZCliShowArgs {
} ZCliShowArgs;

typedef struct ZCliSwitchArgs {
    ZapVersion version;
    bool local;
} ZCliSwitchArgs;

typedef struct ZCliWhichArgs {
    ZapVersion version; // optional
    ZapToolchainElement tool;
} ZCliWhichArgs;

typedef struct ZCliTestArgs {
    ZapVersion version; // optional
} ZCliTestArgs;

typedef struct ZCliHelpArgs {
    ZCliCommand target;
} ZCliHelpArgs;


typedef struct ZCliGlobalArgs {
    bool ignore_local;
} ZCliGlobalArgs;

typedef struct ZCliArgs {
    ZCliGlobalArgs global_args;
    ZCliCommand cmd;
    union {
        ZCliInstallArgs install;
        ZCliUninstallArgs uninstall;
        ZCliReshimArgs reshim;
        ZCliSwitchArgs switch_;
        ZCliListArgs list;
        ZCliShowArgs show;
        ZCliWhichArgs which;
        ZCliTestArgs test;
        ZCliSyncArgs sync;
        ZCliHelpArgs help;
    } cmd_args;
} ZCliArgs;
