#pragma once

#include <defs/int-types.h>
#include <defs/sv.h>

typedef struct ZHelpFlag {
    ZStringView name;
    ZStringView desc;
} ZHelpFlag;

typedef struct ZHelpCommand {
    ZStringView name;
    ZStringView desc;

    const ZHelpFlag* flags;
} ZHelpCommand;

typedef struct ZHelpInfo {
    ZStringView name;
    ZStringView desc;

    ZHelpFlag* global_flags;
    usize global_flags_count;

    const ZHelpCommand* cmds;
    usize command_count;

    ZStringView footer;
} ZHelpInfo;

extern ZHelpInfo zapup_get_help();

