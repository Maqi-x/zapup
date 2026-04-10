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
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#if Z_PLATFORM_IS_MACOS
#include <mach-o/dyld.h>
#endif

bool z_path_abs(ZPathView path, ZPathBuf* out_abs) {
    if (path.len == 0) return false;

    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    char* abs_path = realpath(cpath, NULL);
    if (abs_path) {
        z_strbuf_clear(out_abs);
        bool success = z_strbuf_append(out_abs, z_sv_from_cstr(abs_path));
        free(abs_path);
        free(cpath);
        return success;
    }

    if (z_pathview_is_absolute(path)) {
        z_strbuf_clear(out_abs);
        bool success = z_strbuf_append(out_abs, path);
        free(cpath);
        return success;
    }

    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        free(cpath);
        return false;
    }

    z_strbuf_clear(out_abs);
    if (!z_strbuf_append(out_abs, z_sv_from_cstr(cwd))) {
        free(cpath);
        return false;
    }
    bool success = z_pathbuf_join(out_abs, path);
    free(cpath);
    return success;
}

bool z_read_file(ZPathView path, ZStringBuf* out) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    int fd = open(cpath, O_RDONLY);
    free(cpath);
    if (fd == -1) return false;

    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return false;
    }

    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return false;
    }

    usize size = (usize)st.st_size;
    if (!z_strbuf_resize(out, size)) {
        close(fd);
        return false;
    }

    ssize_t nread = read(fd, out->data, size);
    close(fd);

    if (nread == -1 || (usize)nread != size) {
        return false;
    }

    return true;
}

bool z_write_file(ZPathView path, ZStringView content) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    int fd = open(cpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    free(cpath);
    if (fd == -1) return false;

    ssize_t nwritten = write(fd, content.data, content.len);
    close(fd);

    return nwritten != -1 && (usize)nwritten == content.len;
}

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

bool z_mkdir_if_not_exists(ZPathView path) {
    return z_mkdir(path);
}

bool z_mkfile_if_not_exists(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    int fd = open(cpath, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (fd == -1) {
        if (errno == EEXIST) {
            free(cpath);
            return true;
        }
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

bool z_rm(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    int res = remove(cpath);
    free(cpath);
    return res == 0;
}

static bool z_rm_recursive_internal(ZPathBuf* pb) {
    ZPathView current_pv = z_pathbuf_as_view(pb);
    char* cpath = z_sv_to_cstr_alloc(current_pv);
    if (!cpath) return false;

    struct stat st;
    if (lstat(cpath, &st) != 0) {
        free(cpath);
        return false;
    }

    if (!S_ISDIR(st.st_mode)) {
        int res = remove(cpath);
        free(cpath);
        return res == 0;
    }

    DIR* dir = opendir(cpath);
    free(cpath);
    if (!dir) return false;

    bool success = true;
    struct dirent* entry;
    usize original_len = pb->len;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!z_pathbuf_join(pb, z_sv_from_cstr(entry->d_name))) {
            success = false;
            break;
        }

        if (!z_rm_recursive_internal(pb)) {
            success = false;
        }

        pb->len = original_len;
        if (!success) break;
    }

    closedir(dir);
    if (success) {
        char* dpath = z_sv_to_cstr_alloc(z_pathbuf_as_view(pb));
        if (dpath) {
            success = (rmdir(dpath) == 0);
            free(dpath);
        } else {
            success = false;
        }
    }
    return success;
}

bool z_rm_recursive(ZPathView path) {
    if (path.len == 0) return false;
    ZPathBuf pb;
    if (!z_pathbuf_init_from(&pb, path)) return false;
    bool res = z_rm_recursive_internal(&pb);
    z_pathbuf_destroy(&pb);
    return res;
}

bool z_file_exists(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    struct stat st;
    int res = stat(cpath, &st);
    free(cpath);
    return res == 0;
}

ZFileType z_get_file_type(ZPathView path) {
    if (path.len == 0) return Z_FILE_NOT_FOUND;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return Z_FILE_NOT_FOUND;

    struct stat st;
    if (lstat(cpath, &st) != 0) {
        free(cpath);
        return Z_FILE_NOT_FOUND;
    }

    free(cpath);
    if (S_ISREG(st.st_mode)) return Z_FILE_REGULAR;
    if (S_ISDIR(st.st_mode)) return Z_FILE_DIR;
    if (S_ISLNK(st.st_mode)) return Z_FILE_SYMLINK;
    return Z_FILE_OTHER;
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

bool z_get_self_executable(ZPathBuf* out) {
    if (!out) return false;

#if Z_PLATFORM_IS_MACOS
    uint32_t size = 0;
    if (_NSGetExecutablePath(NULL, &size) == 0) {
        return false;
    }

    char* buf = (char*)malloc(size);
    if (!buf) return false;

    if (_NSGetExecutablePath(buf, &size) != 0) {
        free(buf);
        return false;
    }

    char* real = realpath(buf, NULL);
    z_strbuf_clear(out);
    bool success = false;
    if (real) {
        success = z_strbuf_append(out, z_sv_from_cstr(real));
        free(real);
    } else {
        success = z_strbuf_append(out, z_sv_from_cstr(buf));
    }

    free(buf);
    return success;
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        z_strbuf_clear(out);
        return z_strbuf_append(out, z_sv_from_cstr(buf));
    }

    char* real = realpath("/proc/self/exe", NULL);
    if (real) {
        z_strbuf_clear(out);
        bool ok = z_strbuf_append(out, z_sv_from_cstr(real));
        free(real);
        return ok;
    }

    return false;
#endif
}

#endif // Z_PLATFORM_IS_POSIX
