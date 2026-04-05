#include <zapup/clone.h>

int main() {
    git_libgit2_init();

    ZResolvableZapVersion v = {
        .branch = Z_SV("main"),
        .commit = Z_SV("be6ca9a3e365b5d9568ea60db16d42b94b83e136"),
    };

    git_repository* repo;
    z_clone_zap_repo_with_version(v, Z_PV("./out-repo"), &repo);

    git_libgit2_shutdown();
}
