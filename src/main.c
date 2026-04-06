#include <zapup/cli/parse.h>
#include <zapup/cli/args.h>

#include <zapup/clone.h>

int main(int argc, const char* const* argv) {
    git_libgit2_init();

    ZCliArgs args;
    ZCliParseResult err = z_cli_parse_args(argc, argv, &args);
    if (err.code != Z_CLI_PARSE_OK) {
        printf("error: %d\n", err.code);
    }

    switch (args.cmd) {
    case Z_CLI_CMD_INSTALL: {
        puts("installing");
        ZResolvableZapVersion v = args.cmd_args.install.version;
        git_repository* repo;
        int res = z_clone_zap_repo_with_version(v, Z_PV("./out-repo"), &repo);
        if (res != 0) {
            const git_error* err = git_error_last();
            printf("error %d: %s\n", res, err->message);
        }
        git_repository_free(repo);
        break;
    }
    case Z_CLI_CMD_UNINSTALL:
        puts("uninstall: not implemented yet");
        break;
    case Z_CLI_CMD_SYNC:
        puts("sync: not implemented yet");
        break;
    case Z_CLI_CMD_HELP:
        puts("help: not implemented yet");
        break;
    case Z_CLI_CMD_UNKNOWN:
        puts("unknown command");
        break;
    }

    git_libgit2_shutdown();
}
