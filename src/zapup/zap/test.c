#include <zapup/zap/test.h>

#include <util/pathview.h>
#include <util/pathbuf.h>
#include <util/cmd.h>
#include <defs/sv.h>

#include <stdbool.h>

bool z_run_zap_tests(ZapVersion ver, ZPathView path, bool* out_all_tests_passed) {
    (void) ver; // maybe needed in the future
    if (out_all_tests_passed) {
        *out_all_tests_passed = false;
    }

    ZPathBuf run_tests_sh_path;
    z_pathbuf_init_from(&run_tests_sh_path, path);
    z_pathbuf_join(&run_tests_sh_path, Z_PV("run_tests.sh"));

    ZCommand test_command = {
        .cwd = path,
        .argv = Z_STRING_VIEWS(z_pathbuf_as_view(&run_tests_sh_path)),
    };

    ZCmdRunResult result = z_cmd_run(&test_command);
    if (result.status != Z_CMD_OK) {
        return false;
    } else {
        if (out_all_tests_passed) {
            *out_all_tests_passed = result.exit_code == 0;
        }
        return true;
    }
}
