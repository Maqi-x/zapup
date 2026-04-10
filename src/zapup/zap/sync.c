#include <zapup/zap/sync.h>
#include <zapup/zap/stable.h>

#include <git2.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <defs/sv.h>
#include <util/pathview.h>

static inline bool is_dynamic_revspec(ZapVersion ver) {
     return ver.ref_kind == Z_REF_LATEST;
}

static int open_repository_at_path(ZPathView path, git_repository** out_repo) {
    char* path_cstr = z_sv_to_cstr_alloc(path);
    if (!path_cstr) return -1;
    int err = git_repository_open(out_repo, path_cstr);
    free(path_cstr);
    return err;
}

static int fetch_origin_if_exists(git_repository* repo) {
    git_remote* remote = NULL;
    int err = git_remote_lookup(&remote, repo, "origin");
    if (err != 0) {
        giterr_clear();
        return 0;
    }

    err = git_remote_fetch(remote, NULL, NULL, NULL);
    git_remote_free(remote);
    return err;
}

static int resolve_head_ref(git_repository* repo, git_reference** out_resolved) {
    *out_resolved = NULL;
    git_reference* head_ref = NULL;
    int err = git_repository_head(&head_ref, repo);
    if (err != 0) {
        giterr_clear();
        return 0;
    }

    err = git_reference_resolve(out_resolved, head_ref);
    git_reference_free(head_ref);
    return err;
}

static char* build_remote_tracking_refname(const char* resolved_name) {
    const char* heads_prefix = "refs/heads/";
    const char* remotes_prefix = "refs/remotes/origin/";

    size_t hp_len = strlen(heads_prefix);
    if (strncmp(resolved_name, heads_prefix, hp_len) != 0) {
        return NULL;
    }

    const char* branch_short = resolved_name + hp_len;
    size_t out_len = strlen(remotes_prefix) + strlen(branch_short) + 1;
    char* out = malloc(out_len);
    if (!out) return NULL;
    memcpy(out, remotes_prefix, strlen(remotes_prefix));
    memcpy(out + strlen(remotes_prefix), branch_short, strlen(branch_short));
    out[out_len - 1] = '\0';
    return out;
}

static int lookup_ref_if_exists(git_repository* repo, const char* refname, git_reference** out_ref) {
    *out_ref = NULL;
    int err = git_reference_lookup(out_ref, repo, refname);
    if (err != 0) {
        giterr_clear();
        return 0;
    }
    return 0;
}

static int compare_ref_oids(git_reference* local, git_reference* remote) {
    const git_oid* l = git_reference_target(local);
    const git_oid* r = git_reference_target(remote);
    if (!l || !r) return -1;
    return (git_oid_cmp(l, r) == 0) ? 0 : 1;
}

static int checkout_and_move_ref(git_repository* repo, const git_oid* remote_oid, git_reference* local_resolved, git_reference** out_moved_ref) {
    *out_moved_ref = NULL;
    git_object* commit_obj = NULL;
    int err = git_object_lookup(&commit_obj, repo, remote_oid, GIT_OBJECT_COMMIT);
    if (err != 0) return err;

    git_checkout_options checkout_opts;
    git_checkout_options_init(&checkout_opts, GIT_CHECKOUT_OPTIONS_VERSION);
    checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

    err = git_checkout_tree(repo, commit_obj, &checkout_opts);
    if (err != 0) {
        git_object_free(commit_obj);
        return err;
    }

    err = git_reference_set_target(out_moved_ref, local_resolved, remote_oid, "zapup: fast-forward");
    git_object_free(commit_obj);
    return err;
}

int z_sync_zap_repo_with_version(ZapVersion ver, ZPathView path, git_repository** out_repo, bool* out_updated) {
    int err = 0;
    git_repository* repo = NULL;
    git_reference* resolved_head = NULL;
    git_reference* remote_ref = NULL;
    git_reference* moved_ref = NULL;
    char* remote_ref_name = NULL;
    char* branch_short_cstr = NULL;
    char* local_branch_refname = NULL;
    git_reference* local_branch_ref = NULL;
    bool detached_mode = false;

    if (out_updated) *out_updated = false;

    if (!is_dynamic_revspec(ver)) {
        if (out_repo) *out_repo = NULL;
        return 0;
    }

    err = open_repository_at_path(path, &repo);
    if (err != 0) {
        return err;
    }

    err = fetch_origin_if_exists(repo);
    if (err != 0) goto cleanup;

    err = resolve_head_ref(repo, &resolved_head);
    if (err != 0) goto cleanup;
    if (resolved_head == NULL) {
        err = 0;
        goto cleanup;
    }

    if (ver.ref_kind == Z_REF_STABLE) {
        git_oid stable_oid;
        int serr = z_stable_resolve_best_tag_commit(repo, &stable_oid);
        if (serr == 1) {
            err = 0;
            goto cleanup;
        }
        if (serr != 0) {
            err = serr;
            goto cleanup;
        }

        const git_oid* head_oid = git_reference_target(resolved_head);
        if (head_oid && git_oid_cmp(head_oid, &stable_oid) == 0) {
            if (out_updated) *out_updated = false;
            err = 0;
            goto cleanup;
        }

        git_object* commit_obj = NULL;
        err = git_object_lookup(&commit_obj, repo, &stable_oid, GIT_OBJECT_COMMIT);
        if (err != 0) goto cleanup;

        git_checkout_options checkout_opts;
        git_checkout_options_init(&checkout_opts, GIT_CHECKOUT_OPTIONS_VERSION);
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

        err = git_checkout_tree(repo, commit_obj, &checkout_opts);
        if (err != 0) {
            git_object_free(commit_obj);
            goto cleanup;
        }

        err = git_repository_set_head_detached(repo, &stable_oid);
        git_object_free(commit_obj);
        if (err != 0) goto cleanup;

        if (out_updated) *out_updated = true;
        goto cleanup;
    }

    {
        const char* resolved_name = git_reference_name(resolved_head);
        remote_ref_name = build_remote_tracking_refname(resolved_name);
        if (!remote_ref_name) {
            const char* branch_short = NULL;
            if (!z_sv_is_null(ver.branch)) {
                branch_short_cstr = z_sv_to_cstr_alloc(ver.branch);
                if (!branch_short_cstr) { err = -1; goto cleanup; }
                branch_short = branch_short_cstr;
            } else {
                branch_short = "main";
            }
            size_t rlen = strlen("refs/remotes/origin/") + strlen(branch_short) + 1;
            remote_ref_name = malloc(rlen);
            if (!remote_ref_name) { err = -1; goto cleanup; }
            memcpy(remote_ref_name, "refs/remotes/origin/", strlen("refs/remotes/origin/"));
            memcpy(remote_ref_name + strlen("refs/remotes/origin/"), branch_short, strlen(branch_short));
            remote_ref_name[rlen - 1] = '\0';

            size_t llen = strlen("refs/heads/") + strlen(branch_short) + 1;
            local_branch_refname = malloc(llen);
            if (!local_branch_refname) { err = -1; goto cleanup; }
            memcpy(local_branch_refname, "refs/heads/", strlen("refs/heads/"));
            memcpy(local_branch_refname + strlen("refs/heads/"), branch_short, strlen(branch_short));
            local_branch_refname[llen - 1] = '\0';

            detached_mode = true;
        }
    }

    err = lookup_ref_if_exists(repo, remote_ref_name, &remote_ref);
    if (err != 0) goto cleanup;
    if (remote_ref == NULL) {
        err = 0;
        goto cleanup;
    }

    if (!detached_mode) {
        int cmp = compare_ref_oids(resolved_head, remote_ref);
        if (cmp == -1) {
            err = 0;
            goto cleanup;
        } else if (cmp == 0) {
            if (out_updated) *out_updated = false;
            err = 0;
            goto cleanup;
        }

        {
            const git_oid* remote_oid = git_reference_target(remote_ref);
            err = checkout_and_move_ref(repo, remote_oid, resolved_head, &moved_ref);
            if (err != 0) goto cleanup;
            if (out_updated) *out_updated = true;
        }
    } else {
        const git_oid* remote_oid = git_reference_target(remote_ref);
        const git_oid* head_oid = git_reference_target(resolved_head);
        if (!remote_oid || !head_oid) { err = 0; goto cleanup; }
        if (git_oid_cmp(head_oid, remote_oid) == 0) {
            if (out_updated) *out_updated = false;
            err = 0;
            goto cleanup;
        }

        git_object* commit_obj = NULL;
        err = git_object_lookup(&commit_obj, repo, remote_oid, GIT_OBJECT_COMMIT);
        if (err != 0) goto cleanup;

        git_checkout_options checkout_opts;
        git_checkout_options_init(&checkout_opts, GIT_CHECKOUT_OPTIONS_VERSION);
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

        err = git_checkout_tree(repo, commit_obj, &checkout_opts);
        if (err != 0) {
            git_object_free(commit_obj);
            goto cleanup;
        }

        err = git_reference_lookup(&local_branch_ref, repo, local_branch_refname);
        if (err != 0) {
            giterr_clear();
            local_branch_ref = NULL;
        }

        if (local_branch_ref) {
            err = git_reference_set_target(&moved_ref, local_branch_ref, remote_oid, "zapup: fast-forward");
        } else {
            err = git_reference_create(&moved_ref, repo, local_branch_refname, remote_oid, 1, "zapup: create branch");
        }
        git_object_free(commit_obj);
        if (err != 0) goto cleanup;

        err = git_repository_set_head(repo, local_branch_refname);
        if (err != 0) goto cleanup;

        if (out_updated) *out_updated = true;
    }

cleanup:
    if (moved_ref) git_reference_free(moved_ref);
    if (remote_ref) git_reference_free(remote_ref);
    if (local_branch_ref) git_reference_free(local_branch_ref);
    if (resolved_head) git_reference_free(resolved_head);
    free(remote_ref_name);
    free(local_branch_refname);
    free(branch_short_cstr);

    if (err != 0) {
        if (repo && out_repo == NULL) {
            git_repository_free(repo);
            repo = NULL;
        }
        return err;
    }

    if (out_repo) {
        *out_repo = repo;
    } else {
        if (repo) git_repository_free(repo);
    }

    return 0;
}
