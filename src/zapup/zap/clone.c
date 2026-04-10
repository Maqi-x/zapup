#include <zapup/zap/clone.h>
#include <zapup/zap/stable.h>

#include <git2.h>
#include <stdlib.h>
#include <string.h>

#define Z_ZAP_REPO_URL "https://github.com/thezaplang/zap.git"

typedef struct CloneProgressPayload {
    ZapCloneProgressCallback callback;
    void* user_data;
} CloneProgressPayload;

static int z_clone_transfer_progress_cb(const git_indexer_progress* stats, void* payload_ptr) {
    if (!stats || !payload_ptr) return 0;

    CloneProgressPayload* payload = (CloneProgressPayload*)payload_ptr;
    if (!payload->callback) return 0;

    int percent = 0;
    if (stats->total_objects > 0) {
        percent = (int)((stats->received_objects * 100U) / stats->total_objects);
    } else if (stats->received_objects > 0) {
        percent = 100;
    }

    ZapCloneProgress progress = {
        .received_objects = stats->received_objects,
        .total_objects = stats->total_objects,
        .indexed_objects = stats->indexed_objects,
        .received_bytes = stats->received_bytes,
        .percent = percent,
    };

    payload->callback(&progress, payload->user_data);
    return 0;
}

int z_clone_zap_repo_with_version_progress(
    ZapVersion ver, ZPathView path, git_repository** out_repo,
    ZapCloneProgressCallback progress_cb, void* progress_user_data
) {
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
    CloneProgressPayload progress_payload = {
        .callback = progress_cb,
        .user_data = progress_user_data,
    };

    if (progress_cb) {
        clone_opts.fetch_opts.callbacks.transfer_progress = z_clone_transfer_progress_cb;
        clone_opts.fetch_opts.callbacks.payload = &progress_payload;
    }

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
        git_oid stable_oid;
        int serr = z_stable_resolve_best_tag_commit(repo, &stable_oid);
        if (serr == 1) {
            error = -1;
            goto cleanup;
        }
        if (serr != 0) {
            error = serr;
            goto cleanup;
        }

        error = git_object_lookup(&commit_obj, repo, &stable_oid, GIT_OBJECT_COMMIT);
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

int z_clone_zap_repo_with_version(ZapVersion ver, ZPathView path, git_repository** out_repo) {
    return z_clone_zap_repo_with_version_progress(ver, path, out_repo, NULL, NULL);
}
