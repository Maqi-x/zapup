#pragma once

#include <stdarg.h>

#if defined(__GNUC__) || defined(__clang__)
#  define Z_PRINTF_FUNCTION(FMT, ARGS) __attribute__((format(printf, FMT, ARGS)))
#else
#  define Z_PRINTF_FUNCTION(FMT, ARGS)
#endif

typedef enum ZOutputType {
    Z_OUTPUT_ERROR,
    Z_OUTPUT_WARN,
    Z_OUTPUT_INFO,
    Z_OUTPUT_DEBUG,
} ZOutputType;

void z_output(ZOutputType type, const char* fmt, ...) Z_PRINTF_FUNCTION(2, 3);
void z_output_valist(ZOutputType type, const char* fmt, va_list args);

void z_show_error(const char* fmt, ...) Z_PRINTF_FUNCTION(1, 2);
void z_show_warn(const char* fmt, ...)  Z_PRINTF_FUNCTION(1, 2);
void z_show_info(const char* fmt, ...)  Z_PRINTF_FUNCTION(1, 2);
void z_show_debug(const char* fmt, ...) Z_PRINTF_FUNCTION(1, 2);
