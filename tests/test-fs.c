#include <unity.h>

#include <util/pathview.h>
#include <util/fs.h>

#define TEST_DIR "test_tmp_fs"

void setUp(void) {
    z_rm_recursive(Z_PV(TEST_DIR));
    z_mkdir_all(Z_PV(TEST_DIR));
}

void tearDown(void) {
    z_rm_recursive(Z_PV(TEST_DIR));
}

void test_fs_mkdir_rm(void) {
    ZPathView path = Z_PV(TEST_DIR "/dir1");
    TEST_ASSERT_FALSE(z_file_exists(path));
    
    TEST_ASSERT_TRUE(z_mkdir(path));
    TEST_ASSERT_TRUE(z_file_exists(path));
    TEST_ASSERT_EQUAL(Z_FILE_DIR, z_get_file_type(path));
    
    TEST_ASSERT_TRUE(z_rm(path));
    TEST_ASSERT_FALSE(z_file_exists(path));
}

void test_fs_mkdir_all(void) {
    ZPathView path = Z_PV(TEST_DIR "/a/b/c");
    TEST_ASSERT_FALSE(z_file_exists(path));
    
    TEST_ASSERT_TRUE(z_mkdir_all(path));
    TEST_ASSERT_TRUE(z_file_exists(path));
    TEST_ASSERT_EQUAL(Z_FILE_DIR, z_get_file_type(Z_PV(TEST_DIR "/a")));
    TEST_ASSERT_EQUAL(Z_FILE_DIR, z_get_file_type(Z_PV(TEST_DIR "/a/b")));
    TEST_ASSERT_EQUAL(Z_FILE_DIR, z_get_file_type(Z_PV(TEST_DIR "/a/b/c")));
}

void test_fs_mkfile(void) {
    ZPathView path = Z_PV(TEST_DIR "/file.txt");
    TEST_ASSERT_FALSE(z_file_exists(path));
    
    TEST_ASSERT_TRUE(z_mkfile(path));
    TEST_ASSERT_TRUE(z_file_exists(path));
    TEST_ASSERT_EQUAL(Z_FILE_REGULAR, z_get_file_type(path));
    
    TEST_ASSERT_TRUE(z_mkfile_if_not_exists(path));
    
    TEST_ASSERT_TRUE(z_rm(path));
    TEST_ASSERT_FALSE(z_file_exists(path));
}

void test_fs_read_write_file(void) {
    ZPathView path = Z_PV(TEST_DIR "/rw_file.txt");
    ZStringView content = Z_SV("hello world\nthis is a test");
    
    TEST_ASSERT_TRUE(z_write_file(path, content));
    TEST_ASSERT_TRUE(z_file_exists(path));
    
    ZStringBuf read_buf;
    z_strbuf_init(&read_buf);
    
    TEST_ASSERT_TRUE(z_read_file(path, &read_buf));
    TEST_ASSERT_TRUE(z_strbuf_eql_to(&read_buf, content));
    
    z_strbuf_destroy(&read_buf);
}

void test_fs_touch(void) {
    ZPathView path = Z_PV(TEST_DIR "/touch_me");
    TEST_ASSERT_FALSE(z_file_exists(path));
    
    TEST_ASSERT_TRUE(z_touch(path));
    TEST_ASSERT_TRUE(z_file_exists(path));
    
    TEST_ASSERT_TRUE(z_touch(path));
    TEST_ASSERT_TRUE(z_file_exists(path));
}

void test_fs_rm_recursive(void) {
    z_mkdir_all(Z_PV(TEST_DIR "/deep/dir/structure"));
    z_mkfile(Z_PV(TEST_DIR "/deep/file1"));
    z_mkfile(Z_PV(TEST_DIR "/deep/dir/file2"));
    
    TEST_ASSERT_TRUE(z_file_exists(Z_PV(TEST_DIR "/deep/dir/structure")));
    
    TEST_ASSERT_TRUE(z_rm_recursive(Z_PV(TEST_DIR "/deep")));
    TEST_ASSERT_FALSE(z_file_exists(Z_PV(TEST_DIR "/deep")));
}

void test_fs_set_executable(void) {
#if Z_PLATFORM_IS_POSIX
    ZPathView path = Z_PV(TEST_DIR "/script.sh");
    z_mkfile(path);
    
    TEST_ASSERT_TRUE(z_set_executable(path, true));
   
    // TODO: maybe we should check that it succeed using stat()

    TEST_ASSERT_TRUE(z_set_executable(path, false));
#endif
}

void test_fs_path_abs(void) {
    ZPathBuf abs;
    z_pathbuf_init(&abs);

    ZPathView rel = Z_PV(".");
    TEST_ASSERT_TRUE(z_path_abs(rel, &abs));
    TEST_ASSERT_TRUE(z_pathview_is_absolute(z_pathbuf_as_view(&abs)));

    z_pathbuf_clear(&abs);
    rel = Z_PV("test_file_that_does_not_exist");
    TEST_ASSERT_TRUE(z_path_abs(rel, &abs));
    TEST_ASSERT_TRUE(z_pathview_is_absolute(z_pathbuf_as_view(&abs)));

    z_pathbuf_clear(&abs);
    rel = Z_PV("include");
    TEST_ASSERT_TRUE(z_path_abs(rel, &abs));
    ZPathView abs_view = z_pathbuf_as_view(&abs);
    TEST_ASSERT_TRUE(z_pathview_is_absolute(abs_view));
    TEST_ASSERT_TRUE(z_sv_ends_with(abs_view, Z_SV("include")));

    z_pathbuf_destroy(&abs);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fs_mkdir_rm);
    RUN_TEST(test_fs_mkdir_all);
    RUN_TEST(test_fs_mkfile);
    RUN_TEST(test_fs_read_write_file);
    RUN_TEST(test_fs_touch);
    RUN_TEST(test_fs_rm_recursive);
    RUN_TEST(test_fs_set_executable);
    RUN_TEST(test_fs_path_abs);
    return UNITY_END();
}
