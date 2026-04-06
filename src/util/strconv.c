#include <util/strconv.h>

#include <stdio.h>

bool z_format_int(int v, ZStringBuf* out) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d", v);
    if (len < 0 || (size_t)len >= sizeof(buf)) {
        return false;
    }
    return z_strbuf_append(out, z_sv_from_data_and_len(buf, (usize)len));
}
