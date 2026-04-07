#pragma once

#include <zapup/zap/version.h>
#include <util/pathview.h>

#include <git2.h>
#include <stdbool.h>

/*
 * Sync the local clone of the zap repository at `path` with its remote.
 *
 * This function is a no-op for fixed revisions (non-dynamic versions). A
 * "dynamic" version is one whose revspec is `HEAD` (for example `latest`
 * is normalized to `HEAD` by the version parser).
 *
 * Parameters:
 *  - ver: the resolved zap version describing branch/revspec/build.
 *  - path: path to the local cloned zap repository.
 *  - out_repo: optional pointer to receive an opened `git_repository*`. If
 *              provided and the call succeeds, the caller is responsible for
 *              freeing the repository with `git_repository_free`.
 *  - out_updated: optional pointer to a bool that will be set to `true` if
 *                 the dynamic ref (e.g. HEAD) was advanced as a result of
 *                 fetching / updating from the remote. If the version is not
 *                 dynamic this will be set to `false`.
 *
 * Return value:
 *  - 0 on success (including the case where no update was necessary).
 *  - Non-zero libgit2 error code on failure.
 *
 * Notes:
 *  - The function will only attempt to update when the provided `ver` is
 *    dynamic (HEAD). For static revisions it intentionally does nothing.
 *  - The caller should treat `out_repo` and `out_updated` as optional and may
 *    pass NULL if those values are not needed.
 */
int z_sync_zap_repo_with_version(ZResolvableZapVersion ver, ZPathView path, git_repository** out_repo, bool* out_updated);