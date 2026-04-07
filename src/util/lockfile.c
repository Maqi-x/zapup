#include <util/lockfile.h>
#include <util/fs.h>
#include <defs/platform.h>

#include <stdio.h>
#include <stdlib.h>

#if Z_PLATFORM_IS_POSIX
#  include <fcntl.h>
#  include <unistd.h>
#  include <errno.h>
#  include <signal.h>
#elif Z_PLATFORM_IS_WINDOWS
#  include <windows.h>
#endif

static bool is_pid_alive(long pid) {
    if (pid <= 0) return false;
#if Z_PLATFORM_IS_POSIX
    return kill((pid_t)pid, 0) == 0 || errno != ESRCH;
#elif Z_PLATFORM_IS_WINDOWS
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
    if (!h) return false;
    DWORD code;
    bool alive = GetExitCodeProcess(h, &code) && code == STILL_ACTIVE;
    CloseHandle(h);
    return alive;
#endif
}

static bool is_lock_stale(ZPathView path) {
    ZStringBuf sb;
    z_strbuf_init(&sb);
    if (!z_read_file(path, &sb)) {
        z_strbuf_destroy(&sb);
        return false;
    }
    z_strbuf_append_char(&sb, '\0');
    char* end;
    long pid = strtol(sb.data, &end, 10);
    if (end == sb.data || *end != '\0') {
        z_strbuf_destroy(&sb);
        return false;
    }
    z_strbuf_destroy(&sb);
    return !is_pid_alive(pid);
}

bool z_lockfile_lock(ZLockFile* lock, ZPathView path) {
    if (lock->is_locked) return false;

    z_pathbuf_clear(&lock->path);
    if (!z_pathbuf_append(&lock->path, path)) return false;

    char* cpath = z_sv_to_cstr_alloc(z_pathbuf_as_view(&lock->path));
    if (!cpath) return false;

#if Z_PLATFORM_IS_POSIX
    int fd = open(cpath, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (fd == -1 && errno == EEXIST && is_lock_stale(path)) {
        z_rm(path);
        fd = open(cpath, O_CREAT | O_WRONLY | O_EXCL, 0644);
    }

    if (fd == -1) {
        free(cpath);
        return false;
    }

    char pid_buf[32];
    int pid_len = snprintf(pid_buf, sizeof(pid_buf), "%d", (int)getpid());
    if (pid_len > 0) (void)write(fd, pid_buf, (size_t)pid_len);
    close(fd);
#elif Z_PLATFORM_IS_WINDOWS
    HANDLE h = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_EXISTS && is_lock_stale(path)) {
        z_rm(path);
        h = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (h == INVALID_HANDLE_VALUE) {
        free(cpath);
        return false;
    }

    char pid_buf[32];
    int pid_len = snprintf(pid_buf, sizeof(pid_buf), "%lu", (unsigned long)GetCurrentProcessId());
    if (pid_len > 0) {
        DWORD written;
        WriteFile(h, pid_buf, (DWORD)pid_len, &written, NULL);
    }
    CloseHandle(h);
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
