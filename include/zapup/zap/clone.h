#pragma once

#include <zapup/zap/version.h>
#include <util/pathview.h>

#include <git2.h>

typedef struct ZapCloneProgress {
    unsigned int received_objects;
    unsigned int total_objects;
    unsigned int indexed_objects;
    unsigned int received_bytes;
    int percent;
} ZapCloneProgress;

typedef void (*ZapCloneProgressCallback)(const ZapCloneProgress* progress, void* user_data);

int z_clone_zap_repo_with_version(ZapVersion ver, ZPathView path, git_repository** out_repo);
int z_clone_zap_repo_with_version_progress(
    ZapVersion ver, ZPathView path, git_repository** out_repo,
    ZapCloneProgressCallback progress_cb, void* progress_user_data
);
