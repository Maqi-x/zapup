#pragma once

#include <zapup/zap/version.h>
#include <util/pathview.h>

#include <git2.h>

int z_clone_zap_repo_with_version(ZapVersion ver, ZPathView path, git_repository** out_repo);
