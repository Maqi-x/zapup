#include <zapup/zap/clone.h>

#include <git2.h>
#include <stdlib.h>

#define Z_ZAP_REPO_URL "https://github.com/thezaplang/zap.git"

int z_clone_zap_repo_with_version(ZapVersion ver, ZPathView path, git_repository** out_repo) {
    int error = 0;
    if (out_repo) *out_repo = NULL;

    char* path_cstr = z_sv_to_cstr_alloc(path);
    char* branch_cstr = NULL;
    char* revspec_cstr = NULL;
    git_object* commit_obj = NULL;
    git_repository* repo = NULL;

    if (!path_cstr) return -1;

    git_clone_options clone_opts;
    git_clone_options_init(&clone_opts, GIT_CLONE_OPTIONS_VERSION);

    if (!z_sv_is_null(ver.branch)) {
        branch_cstr = z_sv_to_cstr_alloc(ver.branch);
        if (!branch_cstr) {
            error = -1;
            goto cleanup;
        }
        clone_opts.checkout_branch = branch_cstr;
    }

    error = git_clone(&repo, Z_ZAP_REPO_URL, path_cstr, &clone_opts);
    if (error != 0) goto cleanup;

    if (!z_sv_is_null(ver.revspec) && !z_sv_eql(ver.revspec, Z_SV("HEAD"))) {
        revspec_cstr = z_sv_to_cstr_alloc(ver.revspec);
        if (!revspec_cstr) {
            error = -1;
            goto cleanup;
        }

        error = git_revparse_single(&commit_obj, repo, revspec_cstr);
        if (error != 0) goto cleanup;

        git_checkout_options checkout_opts;
        git_checkout_options_init(&checkout_opts, GIT_CHECKOUT_OPTIONS_VERSION);
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

        error = git_checkout_tree(repo, commit_obj, &checkout_opts);
        if (error != 0) goto cleanup;

        error = git_repository_set_head_detached(repo, git_object_id(commit_obj));
        if (error != 0) goto cleanup;
    }

    if (out_repo) {
        *out_repo = repo;
        repo = NULL;
    }

cleanup:
    if (repo) git_repository_free(repo);
    git_object_free(commit_obj);
    free(revspec_cstr);
    free(branch_cstr);
    free(path_cstr);

    return error;
}
