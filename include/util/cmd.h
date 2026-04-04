#pragma once

#include <util/strbuf.h>
#include <defs/sv.h>

typedef struct ZCommand {
    ZStringView cwd;

    ZStringView* argv;
    ZStringView* envp;

    ZStringBuf* capture_stdout;
    ZStringBuf* capture_stderr;
} ZCommand;

typedef enum ZCmdStatus {
    Z_CMD_OK,
    Z_CMD_SPAWN_ERROR,
} ZCmdStatus;

typedef struct ZCmdRunResult {
    ZCmdStatus status;
    int exit_code;
} ZCmdRunResult;

ZCmdRunResult z_cmd_run(const ZCommand* cmd);
