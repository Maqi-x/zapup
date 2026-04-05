#pragma once

#include <defs/sv.h>

typedef struct ZResolvableZapVersion {
    ZStringView branch;
    ZStringView commit;
} ZResolvableZapVersion;
