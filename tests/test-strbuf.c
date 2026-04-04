#include <unity.h>
#include <util/strbuf.h>

void setUp(void) {}
void tearDown(void) {}

void test_strbuf_init_destroy(void) {
    ZStringBuf sb;
    TEST_ASSERT_TRUE(z_strbuf_init(&sb));
    TEST_ASSERT_EQUAL(0, sb.len);
    TEST_ASSERT_NOT_NULL(sb.data);
    z_strbuf_destroy(&sb);
}

void test_strbuf_append_cstr(void) {
    ZStringBuf sb;
    z_strbuf_init(&sb);
    TEST_ASSERT_TRUE(z_strbuf_append_cstr(&sb, "hello"));
    TEST_ASSERT_EQUAL(5, sb.len);
    TEST_ASSERT_EQUAL_MEMORY("hello", sb.data, 5);
    
    TEST_ASSERT_TRUE(z_strbuf_append_cstr(&sb, " world"));
    TEST_ASSERT_EQUAL(11, sb.len);
    TEST_ASSERT_EQUAL_MEMORY("hello world", sb.data, 11);
    
    z_strbuf_destroy(&sb);
}

void test_strbuf_clear(void) {
    ZStringBuf sb;
    z_strbuf_init(&sb);
    z_strbuf_append_cstr(&sb, "data");
    z_strbuf_clear(&sb);
    TEST_ASSERT_EQUAL(0, sb.len);
    z_strbuf_destroy(&sb);
}

void test_strbuf_append_char(void) {
    ZStringBuf sb;
    z_strbuf_init(&sb);
    z_strbuf_append_char(&sb, 'a');
    z_strbuf_append_char(&sb, 'b');
    TEST_ASSERT_EQUAL(2, sb.len);
    TEST_ASSERT_EQUAL_MEMORY("ab", sb.data, 2);
    z_strbuf_destroy(&sb);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_strbuf_init_destroy);
    RUN_TEST(test_strbuf_append_cstr);
    RUN_TEST(test_strbuf_clear);
    RUN_TEST(test_strbuf_append_char);
    return UNITY_END();
}
