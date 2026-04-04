#pragma once

#include <defs/int-types.h>
#include <defs/sv.h>

typedef struct ZStringViewArray {
    ZStringView* data;
    usize count;
} ZStringViewArray;

#define Z_STRING_VIEWS(...)                                                     \
    (ZStringViewArray) {                                                        \
        .data = (ZStringView[]) { __VA_ARGS__ },                                \
        .count = sizeof((ZStringView[]) { __VA_ARGS__ }) / sizeof(ZStringView)  \
    }
