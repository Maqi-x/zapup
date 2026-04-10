#include <zapup/output.h>
#include <defs/platform.h>

#include <zapup/cli/ansi.h>

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

static void z_output_meta(ZOutputType type, FILE** out_stream, const char** out_prefix, const char** out_color) {
    FILE* stream = stdout;
    const char* prefix = "";
    const char* color = "";

    switch (type) {
    case Z_OUTPUT_ERROR:
        stream = stderr;
        prefix = "error: ";
        color = BOLD RED;
        break;
    case Z_OUTPUT_WARN:
        stream = stderr;
        prefix = "warning: ";
        color = BOLD YELLOW;
        break;
    case Z_OUTPUT_INFO:
        stream = stdout;
        prefix = "info: ";
        color = BOLD BLUE; 
        break;
    case Z_OUTPUT_DEBUG:
        stream = stdout;
        prefix = "debug: ";
        color = BOLD WHITE; 
        break;
    }

    *out_stream = stream;
    *out_prefix = prefix;
    *out_color = color;
}

static void z_output_write_prefix(FILE* stream, const char* prefix, const char* color) {
    const char* reset = "\033[0m";
    bool use_color = should_use_color(stream);

    if (use_color && color[0] != '\0') {
        fprintf(stream, "%s%s%s", color, prefix, reset);
    } else {
        fprintf(stream, "%s", prefix);
    }
}

void z_output_valist(ZOutputType type, const char* fmt, va_list args) {
    FILE* stream = stdout;
    const char* prefix = "";
    const char* color = "";
    z_output_meta(type, &stream, &prefix, &color);
    z_output_write_prefix(stream, prefix, color);

    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

void z_output(ZOutputType type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    z_output_valist(type, fmt, args);
    va_end(args);
}

void z_output_inline_valist(ZOutputType type, const char* fmt, va_list args) {
    FILE* stream = stdout;
    const char* prefix = "";
    const char* color = "";
    z_output_meta(type, &stream, &prefix, &color);
    fprintf(stream, "\r");
    z_output_write_prefix(stream, prefix, color);
    vfprintf(stream, fmt, args);
    fflush(stream);
}

void z_output_inline(ZOutputType type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    z_output_inline_valist(type, fmt, args);
    va_end(args);
}

void z_output_inline_finish(ZOutputType type) {
    FILE* stream = (type == Z_OUTPUT_ERROR || type == Z_OUTPUT_WARN) ? stderr : stdout;
    fprintf(stream, "\n");
    fflush(stream);
}
