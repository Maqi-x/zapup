#pragma once

#include <unity.h>

#define TEST_ASSERT_EQUAL_SV(expected, actual) \
    do { \
        ZStringView _exp = (expected); \
        ZStringView _act = (actual); \
        TEST_ASSERT_EQUAL_UINT_MESSAGE(_exp.len, _act.len, "ZStringView length mismatch"); \
        if (_exp.len > 0) { \
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(_exp.data, _act.data, _exp.len, "ZStringView data mismatch"); \
        } \
    } while (0)

