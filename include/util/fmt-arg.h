#pragma once

#if defined(__GNUC__) || defined(__clang__)
#  define Z_PRINTF_FUNCTION(FMT, ARGS) __attribute__((format(printf, FMT, ARGS)))
#else
#  define Z_PRINTF_FUNCTION(FMT, ARGS)
#endif

