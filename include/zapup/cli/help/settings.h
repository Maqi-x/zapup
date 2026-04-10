#pragma once

#include <stdbool.h>

#include <defs/int-types.h>
#include <defs/sv.h>
#include <zapup/cli/args.h>

typedef struct ZHelpFlag {
    ZStringView name;
    ZStringView desc;
} ZHelpFlag;

typedef struct ZHelpFlagArray {
    const ZHelpFlag* flags;
    usize count;
} ZHelpFlagArray;

typedef struct ZHelpUsageEntry {
    ZStringView name;
    ZStringView desc;
    bool optional;
} ZHelpUsageEntry;

typedef struct ZHelpUsage {
    const ZHelpUsageEntry* entries;
    usize count;
} ZHelpUsage;

typedef struct ZHelpCommand {
    ZCliCommand cmd;
    ZStringView name;
    ZStringView desc;
    ZHelpUsage usage;

    ZHelpFlagArray flags;
} ZHelpCommand;

typedef struct ZHelpInfo {
    ZStringView name;
    ZStringView desc;

    ZHelpFlagArray global_flags;

    ZHelpFlagArray build_flags;

    const ZHelpCommand* cmds;
    usize command_count;

    ZStringView footer;
} ZHelpInfo;

extern ZHelpInfo zapup_get_help();
const ZHelpCommand* zapup_find_help_command(const ZHelpInfo* info, ZCliCommand cmd);

#define Z_HELP_USAGE(NAME, DESC, OPTIONAL) { Z_SV(NAME), Z_SV(DESC), OPTIONAL }

#define Z_HELP_USAGE_FIELDS(...)                                                 \
    (ZHelpUsage) {                                                               \
        .entries = (const ZHelpUsageEntry[]) { __VA_ARGS__ },                   \
        .count = sizeof((const ZHelpUsageEntry[]) { __VA_ARGS__ }) / sizeof(ZHelpUsageEntry) \
    }

#define Z_HELP_NO_USAGE_FIELDS (ZHelpUsage) { .entries = NULL, .count = 0 }


#define Z_HELP_FLAG_ARRAY_FROM(ARR)                                              \
    (ZHelpFlagArray) {                                                           \
        .flags = (ARR),                                                          \
        .count = sizeof(ARR) / sizeof(*(ARR))                                    \
    }

#define Z_HELP_FLAG_ARRAY(...)                                                   \
    (ZHelpFlagArray) {                                                           \
        .flags = (ZHelpFlag[]) { __VA_ARGS__ },                                  \
        .count = sizeof((ZHelpFlag[]) { __VA_ARGS__ }) / sizeof(ZHelpFlag)       \
    }

#define Z_HELP_NO_FLAGS (ZHelpFlagArray) { .flags = NULL, .count = 0 }

