#pragma once

#include <util/pathview.h>
#include <util/pathbuf.h>

typedef struct ZLockFile {
    ZPathBuf path;
    bool is_locked;
} ZLockFile;

static inline void z_lockfile_init(ZLockFile* lock) {
    z_pathbuf_init(&lock->path);
    lock->is_locked = false;
}

void z_lockfile_destroy(ZLockFile* lock);

bool z_lockfile_lock(ZLockFile* lock, ZPathView path);
void z_lockfile_unlock(ZLockFile* lock);
