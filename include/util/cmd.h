#pragma once

#include <util/strbuf.h>
#include <defs/sv-arr.h>
#include <defs/sv.h>

typedef struct ZCommand {
    ZStringView cwd;

    ZStringViewArray argv;

    ZStringBuf* capture_stdout;
    ZStringBuf* capture_stderr;
} ZCommand;

typedef enum ZCmdStatus {
    Z_CMD_OK,
    Z_CMD_MALLOC_ERROR,
    Z_CMD_LAUNCH_ERROR,
    Z_CMD_CHDIR_ERROR,
    Z_CMD_NOT_FOUND,
    Z_CMD_PERMISSION_DENIED,
    Z_CMD_WAIT_ERROR,
} ZCmdStatus;

typedef struct ZCmdRunResult {
    ZCmdStatus status;
    int exit_code;
} ZCmdRunResult;

ZCmdRunResult z_cmd_run(const ZCommand* cmd);
