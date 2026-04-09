#include <zapup/zap/clone.h>

#include <git2.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define Z_ZAP_REPO_URL "https://github.com/thezaplang/zap.git"

int z_clone_zap_repo_with_version(ZapVersion ver, ZPathView path, git_repository** out_repo) {
    int error = 0;
    if (out_repo) *out_repo = NULL;

    char* path_cstr = z_sv_to_cstr_alloc(path);
    char* branch_cstr = NULL;
    char* revspec_cstr = NULL;
    char* chosen_tag = NULL;
    git_strarray tag_names = {0};
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

    if (ver.ref_kind == Z_REF_REVSPEC && !z_sv_is_null(ver.revspec)) {
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
    } else if (ver.ref_kind == Z_REF_STABLE) {
        int terr = git_tag_list(&tag_names, repo);
        if (terr != 0) {
            error = terr;
            goto cleanup;
        }

        long long best_time = LLONG_MIN;
        const char* best_name = NULL;

        for (size_t ti = 0; ti < tag_names.count; ++ti) {
            const char* tname = tag_names.strings[ti];
            git_object* obj2 = NULL;
            if (git_revparse_single(&obj2, repo, tname) != 0) {
                giterr_clear();
                continue;
            }

            git_object_t type = git_object_type(obj2);
            long long t = LLONG_MIN;

            if (type == GIT_OBJECT_TAG) {
                git_tag* tag = (git_tag*)obj2;
                const git_signature* tagger = git_tag_tagger(tag);
                if (tagger) {
                    t = tagger->when.time;
                } else {
                    git_object* target = NULL;
                    if (git_tag_target(&target, tag) == 0) {
                        if (git_object_type(target) == GIT_OBJECT_COMMIT) {
                            t = git_commit_time((git_commit*)target);
                        }
                        git_object_free(target);
                    }
                }
            } else if (type == GIT_OBJECT_COMMIT) {
                t = git_commit_time((git_commit*)obj2);
            }

            git_object_free(obj2);

            if (t > best_time) {
                best_time = t;
                best_name = tname;
            }
        }

        if (!best_name) {
            error = -1;
            goto cleanup;
        }

        size_t l = strlen(best_name) + 1;
        chosen_tag = malloc(l);
        if (!chosen_tag) {
            error = -1;
            goto cleanup;
        }
        memcpy(chosen_tag, best_name, l);
        revspec_cstr = chosen_tag;

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
    if (tag_names.count) git_strarray_dispose(&tag_names);
    free(revspec_cstr);
    free(branch_cstr);
    free(path_cstr);

    return error;
}
