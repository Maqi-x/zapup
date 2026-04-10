#pragma once

#include <util/fmt-arg.h>

#include <stdarg.h>

typedef enum ZOutputType {
    Z_OUTPUT_ERROR,
    Z_OUTPUT_WARN,
    Z_OUTPUT_INFO,
    Z_OUTPUT_DEBUG,
} ZOutputType;

void z_output(ZOutputType type, const char* fmt, ...) Z_PRINTF_FUNCTION(2, 3);
void z_output_valist(ZOutputType type, const char* fmt, va_list args);

#define z_show_error(...) z_output(Z_OUTPUT_ERROR, __VA_ARGS__)
#define z_show_warn(...)  z_output(Z_OUTPUT_WARN,  __VA_ARGS__)
#define z_show_info(...)  z_output(Z_OUTPUT_INFO,  __VA_ARGS__)

#ifndef ZAPUP_RELEASE
#  define z_show_debug(...) z_output(Z_OUTPUT_DEBUG, __VA_ARGS__)
#else
#  define z_show_debug(...) ((void)0)
#endif
