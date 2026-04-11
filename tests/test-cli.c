#include "utility.h"
#include <unity.h>

#include <zapup/cli/parse.h>
#include <util/arr-len.h>

void setUp(void) {}
void tearDown(void) {}

#define ASSERT_PARSE_OK(argc, argv, out) \
    do { \
        ZCliParseResult res = z_cli_parse_args(argc, (const char* const*)argv, out); \
    if (res.code != Z_CLI_PARSE_OK) { \
            z_parse_result_print(&res, Z_OUTPUT_ERROR); \
        } \
        TEST_ASSERT_EQUAL_UINT_MESSAGE(Z_CLI_PARSE_OK, res.code, "CLI parse failed"); \
    } while (0)

#define ASSERT_PARSE_ERR(expected_code, argc, argv) \
    do { \
        ZCliArgs args; \
        ZCliParseResult res = z_cli_parse_args(argc, (const char* const*)argv, &args); \
        TEST_ASSERT_EQUAL_UINT_MESSAGE(expected_code, res.code, "CLI parse did not return expected error code"); \
    } while (0)

// Happy Cases
void test_parse_install() {
    const char* argv[] = { "zapup", "install", "v0.1.1" };
    ZCliArgs args;
    ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
    
    TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_INSTALL, args.cmd);
    TEST_ASSERT_EQUAL_INT(Z_REF_REVSPEC, args.cmd_args.install.version.ref_kind);
    TEST_ASSERT_EQUAL_SV(Z_SV("v0.1.1"), args.cmd_args.install.version.revspec);
}

void test_parse_install_flags() {
    const char* argv[] = { "zapup", "install", "latest", "-j4", "--test" };
    ZCliArgs args;
    ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
    
    TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_INSTALL, args.cmd);
    TEST_ASSERT_EQUAL_INT(Z_REF_LATEST, args.cmd_args.install.version.ref_kind);
    TEST_ASSERT_TRUE(args.cmd_args.install.build.parallel);
    TEST_ASSERT_EQUAL_INT(4, args.cmd_args.install.build.max_jobs);
    TEST_ASSERT_TRUE(args.cmd_args.install.build.run_tests);
}

void test_parse_uninstall() {
    const char* argv[] = { "zapup", "uninstall", "v0.1.1" };
    ZCliArgs args;
    ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
    
    TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_UNINSTALL, args.cmd);
    TEST_ASSERT_EQUAL_INT(Z_REF_REVSPEC, args.cmd_args.uninstall.version.ref_kind);
    TEST_ASSERT_EQUAL_SV(Z_SV("v0.1.1"), args.cmd_args.uninstall.version.revspec);
}

void test_parse_switch() {
    const char* argv[] = { "zapup", "switch", "v0.1.1" };
    ZCliArgs args;
    ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
    
    TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_SWITCH, args.cmd);
    TEST_ASSERT_EQUAL_INT(Z_REF_REVSPEC, args.cmd_args.switch_.version.ref_kind);
    TEST_ASSERT_EQUAL_SV(Z_SV("v0.1.1"), args.cmd_args.switch_.version.revspec);
}

void test_parse_reshim() {
    {
        const char* argv[] = { "zapup", "reshim" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_RESHIM, args.cmd);
        TEST_ASSERT_EQUAL_INT(Z_TOOLCHAIN_ELEMENT_UNKNOWN, args.cmd_args.reshim.tool);
    }
    {
        const char* argv[] = { "zapup", "reshim", "zapc" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_RESHIM, args.cmd);
        TEST_ASSERT_EQUAL_INT(Z_TOOLCHAIN_ELEMENT_ZAPC, args.cmd_args.reshim.tool);
    }
}

void test_parse_which() {
    {
        const char* argv[] = { "zapup", "which", "zapc" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_WHICH, args.cmd);
        TEST_ASSERT_TRUE(z_zap_ver_is_null(args.cmd_args.which.version));
        TEST_ASSERT_EQUAL_INT(Z_TOOLCHAIN_ELEMENT_ZAPC, args.cmd_args.which.tool);
    }
    {
        const char* argv[] = { "zapup", "which", "v0.1.1", "zap-lsp" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_WHICH, args.cmd);
        TEST_ASSERT_EQUAL_INT(Z_REF_REVSPEC, args.cmd_args.which.version.ref_kind);
        TEST_ASSERT_EQUAL_SV(Z_SV("v0.1.1"), args.cmd_args.which.version.revspec);
        TEST_ASSERT_EQUAL_INT(Z_TOOLCHAIN_ELEMENT_ZAP_LSP, args.cmd_args.which.tool);
    }
}

void test_parse_list() {
    const char* argv[] = { "zapup", "list" };
    ZCliArgs args;
    ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
    TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_LIST, args.cmd);
}

void test_parse_show() {
    const char* argv[] = { "zapup", "show" };
    ZCliArgs args;
    ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
    TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_SHOW, args.cmd);
}

void test_parse_help() {
    {
        const char* argv[] = { "zapup", "help" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_HELP, args.cmd);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_UNKNOWN, args.cmd_args.help.target);
    }
    {
        const char* argv[] = { "zapup", "help", "install" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_HELP, args.cmd);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_INSTALL, args.cmd_args.help.target);
    }
}

void test_parse_sync() {
    {
        const char* argv[] = { "zapup", "sync" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_SYNC, args.cmd);
        TEST_ASSERT_TRUE(z_zap_ver_is_null(args.cmd_args.sync.version));
    }
}

void test_parse_test() {
    {
        const char* argv[] = { "zapup", "test" };
        ZCliArgs args;
        ASSERT_PARSE_OK(Z_ARRAY_LEN(argv), argv, &args);
        TEST_ASSERT_EQUAL_INT(Z_CLI_CMD_TEST, args.cmd);
        TEST_ASSERT_TRUE(z_zap_ver_is_null(args.cmd_args.test.version));
    }
}

// --- Non-Happy Cases ---
void test_parse_unknown_command() {
    const char* argv[] = { "zapup", "invalid-cmd" };
    ASSERT_PARSE_ERR(Z_CLI_PARSE_UNKNOWN_COMMAND, Z_ARRAY_LEN(argv), argv);
}

void test_parse_no_command() {
    const char* argv[] = { "zapup" };
    ASSERT_PARSE_ERR(Z_CLI_PARSE_COMMAND_EXPECTED, Z_ARRAY_LEN(argv), argv);
}

void test_parse_missing_version() {
    {
        const char* argv[] = { "zapup", "install" };
        ASSERT_PARSE_ERR(Z_CLI_PARSE_MISSING_POSITIONAL_ARG, Z_ARRAY_LEN(argv), argv);
    }
    {
        const char* argv[] = { "zapup", "uninstall" };
        ASSERT_PARSE_ERR(Z_CLI_PARSE_MISSING_POSITIONAL_ARG, Z_ARRAY_LEN(argv), argv);
    }
    {
        const char* argv[] = { "zapup", "switch" };
        ASSERT_PARSE_ERR(Z_CLI_PARSE_MISSING_POSITIONAL_ARG, Z_ARRAY_LEN(argv), argv);
    }
}

void test_parse_missing_tool() {
    const char* argv[] = { "zapup", "which" };
    ASSERT_PARSE_ERR(Z_CLI_PARSE_MISSING_POSITIONAL_ARG, Z_ARRAY_LEN(argv), argv);
}

void test_parse_unknown_flag() {
    {
        const char* argv[] = { "zapup", "install", "v0.1.1", "--unknown" };
        ASSERT_PARSE_ERR(Z_CLI_PARSE_UNKNOWN_LONG_FLAG, Z_ARRAY_LEN(argv), argv);
    }
    {
        const char* argv[] = { "zapup", "install", "v0.1.1", "-u" };
        ASSERT_PARSE_ERR(Z_CLI_PARSE_UNKNOWN_SHORT_FLAG, Z_ARRAY_LEN(argv), argv);
    }
}

void test_parse_wrong_arg_format() {
    const char* argv[] = { "zapup", "sync", "--parallel=abc" };
    ASSERT_PARSE_ERR(Z_CLI_PARSE_WRONG_ARG_FORMAT, Z_ARRAY_LEN(argv), argv);
}

void test_parse_unexpected_arg() {
    const char* argv[] = { "zapup", "switch", "v0.1.1", "extra" };
    ASSERT_PARSE_ERR(Z_CLI_PARSE_UNEXPECTED_ARG, Z_ARRAY_LEN(argv), argv);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_parse_install);
    RUN_TEST(test_parse_install_flags);
    RUN_TEST(test_parse_uninstall);
    RUN_TEST(test_parse_switch);
    RUN_TEST(test_parse_reshim);
    RUN_TEST(test_parse_which);
    RUN_TEST(test_parse_list);
    RUN_TEST(test_parse_show);
    RUN_TEST(test_parse_help);
    RUN_TEST(test_parse_sync);
    RUN_TEST(test_parse_test);

    RUN_TEST(test_parse_unknown_command);
    RUN_TEST(test_parse_no_command);
    RUN_TEST(test_parse_missing_version);
    RUN_TEST(test_parse_missing_tool);
    RUN_TEST(test_parse_unknown_flag);
    RUN_TEST(test_parse_wrong_arg_format);
    RUN_TEST(test_parse_unexpected_arg);

    return UNITY_END();
}
