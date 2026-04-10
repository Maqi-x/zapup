#include <zapup/zap/stable.h>

#include <git2.h>
#include <limits.h>
#include <string.h>

int z_stable_resolve_best_tag_commit(git_repository* repo, git_oid* out_commit_oid) {
    if (!repo || !out_commit_oid) return -1;

    git_strarray tag_names = {0};
    int err = git_tag_list(&tag_names, repo);
    if (err != 0) return err;

    long long best_time = LLONG_MIN;
    const char* best_name = NULL;
    git_oid best_oid;

    for (size_t ti = 0; ti < tag_names.count; ++ti) {
        const char* tname = tag_names.strings[ti];
        git_object* obj = NULL;
        if (git_revparse_single(&obj, repo, tname) != 0) {
            giterr_clear();
            continue;
        }

        git_object_t type = git_object_type(obj);
        long long t = LLONG_MIN;
        git_oid target_oid;

        if (type == GIT_OBJECT_TAG) {
            git_tag* tag = (git_tag*)obj;
            const git_signature* tagger = git_tag_tagger(tag);
            if (tagger) {
                t = (long long)tagger->when.time;
                git_oid_cpy(&target_oid, git_tag_target_id(tag));
            } else {
                git_object* target = NULL;
                if (git_tag_target(&target, tag) == 0 && target) {
                    if (git_object_type(target) == GIT_OBJECT_COMMIT) {
                        t = (long long)git_commit_time((git_commit*)target);
                        git_oid_cpy(&target_oid, git_commit_id((git_commit*)target));
                    }
                    git_object_free(target);
                }
            }
        } else if (type == GIT_OBJECT_COMMIT) {
            t = (long long)git_commit_time((git_commit*)obj);
            git_oid_cpy(&target_oid, git_commit_id((git_commit*)obj));
        }

        git_object_free(obj);

        if (t > best_time) {
            best_time = t;
            best_name = tname;
            git_oid_cpy(&best_oid, &target_oid);
        }
    }

    git_strarray_dispose(&tag_names);

    if (!best_name) return 1;

    git_oid_cpy(out_commit_oid, &best_oid);
    return 0;
}
