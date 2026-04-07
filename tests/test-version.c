#include <unity.h>
#include "utility.h"

#include <zapup/zap/version.h>

void setUp(void) {}
void tearDown(void) {}

void test_parse_commit_only(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("29be27b"));
    TEST_ASSERT_TRUE(z_sv_is_null(v.branch));
    TEST_ASSERT_EQUAL_SV(Z_SV("29be27b"), v.commit);
    TEST_ASSERT_EQUAL_INT(Z_BUILD_RELEASE, v.build);
}

void test_parse_branch_and_commit(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("main@HEAD"));
    TEST_ASSERT_EQUAL_SV(Z_SV("main"), v.branch);
    TEST_ASSERT_EQUAL_SV(Z_SV("HEAD"), v.commit);
    TEST_ASSERT_EQUAL_INT(Z_BUILD_RELEASE, v.build);
}

void test_parse_commit_release(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("7891981:release"));
    TEST_ASSERT_TRUE(z_sv_is_null(v.branch));
    TEST_ASSERT_EQUAL_SV(Z_SV("7891981"), v.commit);
    TEST_ASSERT_EQUAL_INT(Z_BUILD_RELEASE, v.build);
}

void test_parse_commit_debug(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("c0200e9:debug"));
    TEST_ASSERT_TRUE(z_sv_is_null(v.branch));
    TEST_ASSERT_EQUAL_SV(Z_SV("c0200e9"), v.commit);
    TEST_ASSERT_EQUAL_INT(Z_BUILD_DEBUG, v.build);
}

void test_parse_branch_commit_debug(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("main@HEAD:debug"));
    TEST_ASSERT_EQUAL_SV(Z_SV("main"), v.branch);
    TEST_ASSERT_EQUAL_SV(Z_SV("HEAD"), v.commit);
    TEST_ASSERT_EQUAL_INT(Z_BUILD_DEBUG, v.build);
}

void test_parse_branch_commit_release(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("feat@ea1f166:release"));
    TEST_ASSERT_EQUAL_SV(Z_SV("feat"), v.branch);
    TEST_ASSERT_EQUAL_SV(Z_SV("ea1f166"), v.commit);
    TEST_ASSERT_EQUAL_INT(Z_BUILD_RELEASE, v.build);
}

void test_parse_invalid_empty(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV(""));
    TEST_ASSERT_TRUE(z_zap_ver_is_null(v));
}

void test_parse_invalid_suffix_only(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV(":debug"));
    TEST_ASSERT_TRUE(z_zap_ver_is_null(v));
}

void test_parse_invalid_branch_only(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("main@"));
    TEST_ASSERT_TRUE(z_zap_ver_is_null(v));
}

void test_parse_invalid_at_only(void) {
    ZResolvableZapVersion v = z_parse_zap_version(Z_SV("@"));
    TEST_ASSERT_TRUE(z_zap_ver_is_null(v));
}

void test_format_commit_only(void) {
    ZResolvableZapVersion v = { .branch = Z_SV_NULL, .commit = Z_SV("29be27b"), .build = Z_BUILD_RELEASE };
    ZStringBuf sb;
    z_strbuf_init(&sb);
    z_format_zap_version(v, &sb);
    TEST_ASSERT_EQUAL_SV(Z_SV("29be27b:release"), z_strbuf_view(&sb));
    z_strbuf_destroy(&sb);
}

void test_format_branch_and_commit(void) {
    ZResolvableZapVersion v = { .branch = Z_SV("main"), .commit = Z_SV("HEAD"), .build = Z_BUILD_RELEASE };
    ZStringBuf sb;
    z_strbuf_init(&sb);
    z_format_zap_version(v, &sb);
    TEST_ASSERT_EQUAL_SV(Z_SV("main@HEAD:release"), z_strbuf_view(&sb));
    z_strbuf_destroy(&sb);
}

void test_format_commit_debug(void) {
    ZResolvableZapVersion v = { .branch = Z_SV_NULL, .commit = Z_SV("c0200e9"), .build = Z_BUILD_DEBUG };
    ZStringBuf sb;
    z_strbuf_init(&sb);
    z_format_zap_version(v, &sb);
    TEST_ASSERT_EQUAL_SV(Z_SV("c0200e9:debug"), z_strbuf_view(&sb));
    z_strbuf_destroy(&sb);
}

void test_format_branch_commit_debug(void) {
    ZResolvableZapVersion v = { .branch = Z_SV("main"), .commit = Z_SV("HEAD"), .build = Z_BUILD_DEBUG };
    ZStringBuf sb;
    z_strbuf_init(&sb);
    z_format_zap_version(v, &sb);
    TEST_ASSERT_EQUAL_SV(Z_SV("main@HEAD:debug"), z_strbuf_view(&sb));
    z_strbuf_destroy(&sb);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_parse_commit_only);
    RUN_TEST(test_parse_branch_and_commit);
    RUN_TEST(test_parse_commit_release);
    RUN_TEST(test_parse_commit_debug);
    RUN_TEST(test_parse_branch_commit_debug);
    RUN_TEST(test_parse_branch_commit_release);
    RUN_TEST(test_parse_invalid_empty);
    RUN_TEST(test_parse_invalid_suffix_only);
    RUN_TEST(test_parse_invalid_branch_only);
    RUN_TEST(test_parse_invalid_at_only);

    RUN_TEST(test_format_commit_only);
    RUN_TEST(test_format_branch_and_commit);
    RUN_TEST(test_format_commit_debug);
    RUN_TEST(test_format_branch_commit_debug);

    return UNITY_END();
}
