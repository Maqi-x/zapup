#include <unity.h>

#include <util/cmd.h>
#include <util/strbuf.h>
#include <defs/platform.h>

#include <stdlib.h>

#if Z_PLATFORM_IS_WINDOWS
#  define SHELL "cmd"
#  define SHELL_ARG "/c"
#  define EXIT_CMD "exit"
#else
#  define SHELL "sh"
#  define SHELL_ARG "-c"
#  define EXIT_CMD "exit"
#endif

void setUp(void) {}
void tearDown(void) {}

void test_cmd_run_true(void) {
    ZCommand cmd = {0};
#if Z_PLATFORM_IS_WINDOWS
    cmd.argv = Z_STRING_VIEWS(Z_SV(SHELL), Z_SV(SHELL_ARG), Z_SV(EXIT_CMD), Z_SV("0"));
#else
    cmd.argv = Z_STRING_VIEWS(Z_SV("true"));
#endif

    ZCmdRunResult result = z_cmd_run(&cmd);
    TEST_ASSERT_EQUAL(Z_CMD_OK, result.status);
    TEST_ASSERT_EQUAL(0, result.exit_code);
}

void test_cmd_run_false(void) {
    ZCommand cmd = {0};
#if Z_PLATFORM_IS_WINDOWS
    cmd.argv = Z_STRING_VIEWS(Z_SV(SHELL), Z_SV(SHELL_ARG), Z_SV(EXIT_CMD), Z_SV("1"));
#else
    cmd.argv = Z_STRING_VIEWS(Z_SV("false"));
#endif

    ZCmdRunResult result = z_cmd_run(&cmd);
    TEST_ASSERT_EQUAL(Z_CMD_OK, result.status);
    TEST_ASSERT_NOT_EQUAL(0, result.exit_code);
}

void test_cmd_capture_stdout(void) {
    ZStringBuf stdout_buf;
    z_strbuf_init(&stdout_buf);

    ZCommand cmd = {
#if Z_PLATFORM_IS_WINDOWS
        // On Windows, we use a simple echo. 
        // Note: cmd /c echo hello world -> prints "hello world"
        .argv = Z_STRING_VIEWS(Z_SV(SHELL), Z_SV(SHELL_ARG), Z_SV("echo"), Z_SV("hello")),
#else
        .argv = Z_STRING_VIEWS(Z_SV("echo"), Z_SV("-n"), Z_SV("hello")),
#endif
        .capture_stdout = &stdout_buf,
    };

    ZCmdRunResult result = z_cmd_run(&cmd);
    TEST_ASSERT_EQUAL(Z_CMD_OK, result.status);
    TEST_ASSERT_EQUAL(0, result.exit_code);
    
#if Z_PLATFORM_IS_WINDOWS
    // windows 'echo' adds \r\n
    TEST_ASSERT_TRUE(stdout_buf.len >= 5);
    TEST_ASSERT_EQUAL_MEMORY("hello", stdout_buf.data, 5);
#else
    TEST_ASSERT_EQUAL(5, stdout_buf.len);
    TEST_ASSERT_EQUAL_MEMORY("hello", stdout_buf.data, 5);
#endif

    z_strbuf_destroy(&stdout_buf);
}

void test_cmd_cwd(void) {
    ZStringBuf stdout_buf;
    z_strbuf_init(&stdout_buf);

    const char* temp_dir = getenv(Z_PLATFORM_IS_WINDOWS ? "TEMP" : "TMPDIR");
    if (temp_dir == NULL) temp_dir = "/tmp";

    ZCommand cmd = {
#if Z_PLATFORM_IS_WINDOWS
        .argv = Z_STRING_VIEWS(Z_SV(SHELL), Z_SV(SHELL_ARG), Z_SV("cd")),
#else
        .argv = Z_STRING_VIEWS(Z_SV("pwd")),
#endif
        .cwd = z_sv_from_cstr(temp_dir),
        .capture_stdout = &stdout_buf,
    };

    ZCmdRunResult result = z_cmd_run(&cmd);
    TEST_ASSERT_EQUAL(Z_CMD_OK, result.status);
    TEST_ASSERT_EQUAL(0, result.exit_code);
    TEST_ASSERT_GREATER_THAN(0, stdout_buf.len);

    z_strbuf_destroy(&stdout_buf);
}

void test_cmd_not_found(void) {
    ZCommand cmd = {
        .argv = Z_STRING_VIEWS(Z_SV("non-existent-command-xyz-123")),
    };

    ZCmdRunResult result = z_cmd_run(&cmd);
    TEST_ASSERT_EQUAL(Z_CMD_NOT_FOUND, result.status);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_cmd_run_true);
    RUN_TEST(test_cmd_run_false);
    RUN_TEST(test_cmd_capture_stdout);
    RUN_TEST(test_cmd_cwd);
    RUN_TEST(test_cmd_not_found);
    return UNITY_END();
}
