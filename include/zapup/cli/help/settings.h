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

    const ZHelpFlag* items;
} ZHelpCommand;

typedef struct ZHelpInfo {
    const ZHelpCommand* cmds;
    usize count;
} ZHelpInfo;

extern ZHelpInfo zapup_get_help();

