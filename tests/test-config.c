#include "utility.h"
#include <unity.h>

#include <zapup/config.h>
#include <util/strbuf.h>

void setUp(void) {}
void tearDown(void) {}

void test_config_init_free(void) {
    ZConfig cfg;
    z_config_init(&cfg);

    TEST_ASSERT_TRUE(z_zap_ver_is_null(cfg.toolchain.active_version));
    TEST_ASSERT_EQUAL_SV(Z_SV("cc"), cfg.build.cc);
    TEST_ASSERT_EQUAL_SV(Z_SV("c++"), cfg.build.cxx);
    TEST_ASSERT_NULL(cfg._ctx);

    z_config_free(&cfg);
    TEST_ASSERT_NULL(cfg._ctx);
}

void test_config_from_json(void) {
    ZConfig cfg;
    z_config_init(&cfg);

    const char* json = "{\"build\":{\"cc\":\"clang\",\"cxx\":\"clang++\"}}";
    z_config_from_json(&cfg, z_sv_from_cstr(json));

    TEST_ASSERT_EQUAL_SV(Z_SV("clang"), cfg.build.cc);
    TEST_ASSERT_EQUAL_SV(Z_SV("clang++"), cfg.build.cxx);
    TEST_ASSERT_TRUE(z_zap_ver_is_null(cfg.toolchain.active_version));

    z_config_free(&cfg);
}

void test_config_merge(void) {
    ZConfig cfg;
    z_config_init(&cfg);

    const char* json1 = "{\"build\":{\"cc\":\"clang\"}}";
    z_config_merge_from_json(&cfg, z_sv_from_cstr(json1));

    TEST_ASSERT_EQUAL_SV(Z_SV("clang"), cfg.build.cc);
    TEST_ASSERT_EQUAL_SV(Z_SV("c++"), cfg.build.cxx); // default remains

    const char* json2 = "{\"build\":{\"cxx\":\"g++\"}}";
    z_config_merge_from_json(&cfg, z_sv_from_cstr(json2));

    TEST_ASSERT_EQUAL_SV(Z_SV("clang"), cfg.build.cc);
    TEST_ASSERT_EQUAL_SV(Z_SV("g++"), cfg.build.cxx);

    const char* json3 = "{\"toolchain\":{\"active_version\":{\"branch\":\"main\",\"ref_type\":\"latest\"}}}";
    z_config_merge_from_json(&cfg, z_sv_from_cstr(json3));

    TEST_ASSERT_EQUAL_SV(Z_SV("main"), cfg.toolchain.active_version.branch);
    TEST_ASSERT_EQUAL_INT(Z_REF_LATEST, cfg.toolchain.active_version.ref_kind);
    TEST_ASSERT_EQUAL_SV(Z_SV("clang"), cfg.build.cc);

    z_config_free(&cfg);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_config_init_free);
    RUN_TEST(test_config_from_json);
    RUN_TEST(test_config_merge);
    return UNITY_END();
}
