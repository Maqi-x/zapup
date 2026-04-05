#include <defs/platform.h>

#if Z_PLATFORM_IS_POSIX

#include <util/fs.h>
#include <util/pathview.h>
#include <util/pathbuf.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <utime.h>
#include <stdlib.h>

bool z_mkdir(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    int res = mkdir(cpath, 0755);
    if (res == 0) {
        free(cpath);
        return true;
    }

    if (errno == EEXIST) {
        struct stat st;
        if (stat(cpath, &st) == 0 && S_ISDIR(st.st_mode)) {
            free(cpath);
            return true;
        }
    }

    free(cpath);
    return false;
}

bool z_mkdir_all(ZPathView path) {
    if (path.len == 0) return true;
    if (z_mkdir(path)) return true;

    ZPathView parent = z_pathview_dirname(path);
    if (parent.len == 0 || z_sv_eql(parent, path)) {
        return false;
    }

    if (!z_mkdir_all(parent)) {
        return false;
    }

    return z_mkdir(path);
}

bool z_mkfile(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    int fd = open(cpath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        free(cpath);
        return false;
    }

    close(fd);
    free(cpath);
    return true;
}

bool z_touch(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    int res = utime(cpath, NULL);
    if (res == -1 && errno == ENOENT) {
        int fd = open(cpath, O_CREAT | O_WRONLY, 0644);
        if (fd != -1) {
            close(fd);
            res = 0;
        }
    }

    free(cpath);
    return res == 0;
}

bool z_set_executable(ZPathView path, bool enabled) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    struct stat st;
    if (stat(cpath, &st) != 0) {
        free(cpath);
        return false;
    }

    mode_t mode = st.st_mode;
    if (enabled) {
        mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    } else {
        mode &= ~(S_IXUSR | S_IXGRP | S_IXOTH);
    }

    int res = chmod(cpath, mode);
    free(cpath);
    return res == 0;
}

#endif // Z_PLATFORM_IS_POSIX
