#include <util/lockfile.h>
#include <util/fs.h>
#include <defs/platform.h>

#include <stdio.h>
#include <stdlib.h>

#if Z_PLATFORM_IS_POSIX
#  include <fcntl.h>
#  include <unistd.h>
#elif Z_PLATFORM_IS_WINDOWS
#  include <windows.h>
#endif

bool z_lockfile_lock(ZLockFile* lock, ZPathView path) {
    if (lock->is_locked) return false;

    z_pathbuf_clear(&lock->path);
    if (!z_pathbuf_init_from(&lock->path, path)) return false;

    char* cpath = z_sv_to_cstr_alloc(z_pathbuf_as_view(&lock->path));
    if (!cpath) return false;

#if Z_PLATFORM_IS_POSIX
    int fd = open(cpath, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (fd == -1) {
        free(cpath);
        return false;
    }

    char pid_buf[32];
    int pid_len = snprintf(pid_buf, sizeof(pid_buf), "%d", (int)getpid());
    if (pid_len > 0) {
        (void)write(fd, pid_buf, (size_t)pid_len);
    }
    close(fd);
#elif Z_PLATFORM_IS_WINDOWS
    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        free(cpath);
        return false;
    }

    char pid_buf[32];
    int pid_len = snprintf(pid_buf, sizeof(pid_buf), "%lu", (unsigned long)GetCurrentProcessId());
    if (pid_len > 0) {
        DWORD written;
        WriteFile(hFile, pid_buf, (DWORD)pid_len, &written, NULL);
    }
    CloseHandle(hFile);
#endif

    free(cpath);
    lock->is_locked = true;
    return true;
}

void z_lockfile_unlock(ZLockFile* lock) {
    if (!lock->is_locked) return;

    z_rm(z_pathbuf_as_view(&lock->path));
    lock->is_locked = false;
}

void z_lockfile_destroy(ZLockFile* lock) {
    z_lockfile_unlock(lock);
    z_pathbuf_destroy(&lock->path);
}
