#pragma once

#include <git2.h>

int z_stable_resolve_best_tag_commit(git_repository* repo, git_oid* out_commit_oid);
