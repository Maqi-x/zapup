#include <unity.h>
#include <util/pathbuf.h>

void setUp(void) {}
void tearDown(void) {}

void test_pathbuf_sanitize(void) {
    ZPathBuf pb;
    z_pathbuf_init(&pb);

    z_pathbuf_clear(&pb);
    z_pathbuf_init_from(&pb, Z_PV("my:file*.txt"));
    z_pathbuf_sanitize(&pb);
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, "my-file-.txt"));

    z_pathbuf_clear(&pb);
    z_pathbuf_init_from(&pb, Z_PV("invalid<chars>|in?path\"quote.txt"));
    z_pathbuf_sanitize(&pb);
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, "invalid-chars--in-path-quote.txt"));

    z_pathbuf_clear(&pb);
    z_pathbuf_init_from(&pb, Z_PV("dir:one/file:two.txt"));
    z_pathbuf_sanitize(&pb);
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, "dir-one/file-two.txt"));

#if Z_PLATFORM_IS_WINDOWS
    z_pathbuf_clear(&pb);
    z_pathbuf_init_from(&pb, Z_PV("C:\\dir:one\\file:two.txt"));
    z_pathbuf_sanitize(&pb);
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, "C:\\dir-one\\file-two.txt"));
#endif

    z_pathbuf_clear(&pb);
    char ctrl[] = { 'a', 1, 'b', 31, 'c', '\0' };
    z_pathbuf_init_from(&pb, z_sv_from_cstr(ctrl));
    z_pathbuf_sanitize(&pb);
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, "a-b-c"));

    z_pathbuf_destroy(&pb);
}

void test_pathbuf_pop(void) {
    ZPathBuf pb;
    z_pathbuf_init_from(&pb, Z_PV("a/b/c"));
    
    TEST_ASSERT_TRUE(z_pathbuf_pop(&pb));
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, "a/b"));
    
    TEST_ASSERT_TRUE(z_pathbuf_pop(&pb));
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, "a"));
    
    TEST_ASSERT_TRUE(z_pathbuf_pop(&pb));
    TEST_ASSERT_TRUE(z_strbuf_eql_to_cstr(&pb, ""));

    z_pathbuf_destroy(&pb);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_pathbuf_sanitize);
    RUN_TEST(test_pathbuf_pop);
    return UNITY_END();
}
