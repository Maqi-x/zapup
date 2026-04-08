#pragma once

#include <zapup/zap/version.h>
#include <util/pathview.h>

#include <git2.h>
#include <stdbool.h>

bool z_run_zap_tests(ZResolvableZapVersion ver, ZPathView path, bool* all_tests_passed);
