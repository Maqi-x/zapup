#include <zapup/version.h>

ZResolvableZapVersion z_parse_zap_version(ZStringView s) {
    if (s.len == 0) {
        return Z_ZAP_VERSION_NULL;
    }

    usize sepIndex = s.len;
    for (usize i = 0; i < s.len; ++i) {
        if (s.data[i] == '@') {
            sepIndex = i;
            break;
        }
    }
    
    if (sepIndex == s.len) {
        return (ZResolvableZapVersion) {
            .branch = Z_SV_NULL,
            .commit = s,
        };
    }
    
    return (ZResolvableZapVersion) {
        .branch = z_sv_slice(s, 0, sepIndex),
        .commit = z_sv_slice(s, sepIndex + 1, s.len),
    };
}
