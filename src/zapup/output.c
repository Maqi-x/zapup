#include <zapup/output.h>
#include <defs/platform.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#if Z_PLATFORM_IS_POSIX
#  include <unistd.h>
#elif Z_PLATFORM_IS_WINDOWS
#  include <io.h>
#  define isatty _isatty
#  define fileno _fileno
#endif

static bool should_use_color(FILE* stream) {
    const char* no_color = getenv("NO_COLOR");
    if (no_color && no_color[0] != '\0') {
        return false;
    }

    return isatty(fileno(stream));
}

void z_output_valist(ZOutputType type, const char* fmt, va_list args) {
    FILE* stream = stdout;
    const char* prefix = "";
    const char* color = "";
    const char* reset = "\033[0m";

    bool use_color = false;

    switch (type) {
    case Z_OUTPUT_ERROR:
        stream = stderr;
        prefix = "error: ";
        color = "\033[1;31m"; // bold red
        break;
    case Z_OUTPUT_WARN:
        stream = stderr;
        prefix = "warning: ";
        color = "\033[1;33m"; // bold yellow
        break;
    case Z_OUTPUT_INFO:
        stream = stdout;
        prefix = "info: ";
        color = "\033[1;34m"; // bold glue
        break;
    case Z_OUTPUT_DEBUG:
        stream = stdout;
        prefix = "debug: ";
        color = "\033[1;30m"; // bold gray
        break;
    }

    use_color = should_use_color(stream);

    if (use_color) {
        fprintf(stream, "%s%s%s", color, prefix, reset);
    } else {
        fprintf(stream, "%s", prefix);
    }

    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

void z_output(ZOutputType type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    z_output_valist(type, fmt, args);
    va_end(args);
}
