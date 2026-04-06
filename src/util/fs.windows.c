#include <defs/platform.h>

#if Z_PLATFORM_IS_WINDOWS

#include <util/fs.h>
#include <util/pathview.h>
#include <util/pathbuf.h>

#include <windows.h>
#include <stdlib.h>
#include <string.h>

bool z_path_abs(ZPathView path, ZPathBuf* out_abs) {
    if (path.len == 0) return false;

    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    char buffer[MAX_PATH];
    DWORD res = GetFullPathNameA(cpath, MAX_PATH, buffer, NULL);

    if (res == 0) {
        free(cpath);
        return false;
    }

    if (res >= MAX_PATH) {
        char* heap_buffer = malloc(res);
        if (!heap_buffer) {
            free(cpath);
            return false;
        }
        DWORD res2 = GetFullPathNameA(cpath, res, heap_buffer, NULL);
        free(cpath);
        if (res2 == 0 || res2 >= res) {
            free(heap_buffer);
            return false;
        }
        z_strbuf_clear(out_abs);
        bool success = z_strbuf_append(out_abs, z_sv_from_cstr(heap_buffer));
        free(heap_buffer);
        return success;
    }

    free(cpath);
    z_strbuf_clear(out_abs);
    return z_strbuf_append(out_abs, z_sv_from_cstr(buffer));
}

bool z_read_file(ZPathView path, ZStringBuf* out) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    free(cpath);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return false;
    }

    if (!z_strbuf_resize(out, (usize)size.QuadPart)) {
        CloseHandle(hFile);
        return false;
    }

    DWORD nread;
    BOOL res = ReadFile(hFile, out->data, (DWORD)size.QuadPart, &nread, NULL);
    CloseHandle(hFile);

    if (!res || nread != (DWORD)size.QuadPart) {
        return false;
    }

    return true;
}

bool z_write_file(ZPathView path, ZStringView content) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    free(cpath);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD nwritten;
    BOOL res = WriteFile(hFile, content.data, (DWORD)content.len, &nwritten, NULL);
    CloseHandle(hFile);

    return res && nwritten == (DWORD)content.len;
}

bool z_mkdir(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    BOOL res = CreateDirectoryA(cpath, NULL);
    if (res) {
        free(cpath);
        return true;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        DWORD attrs = GetFileAttributesA(cpath);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
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

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        free(cpath);
        return false;
    }

    CloseHandle(hFile);
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

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_EXISTS || GetLastError() == ERROR_ALREADY_EXISTS) {
            free(cpath);
            return true;
        }
        free(cpath);
        return false;
    }

    CloseHandle(hFile);
    free(cpath);
    return true;
}

bool z_touch(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    HANDLE hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            hFile = CreateFileA(cpath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        }
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        free(cpath);
        return false;
    }

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    
    BOOL res = SetFileTime(hFile, NULL, &ft, &ft);
    
    CloseHandle(hFile);
    free(cpath);
    return res != 0;
}

bool z_rm(ZPathView path) {
    if (path.len == 0) return false;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return false;

    DWORD attrs = GetFileAttributesA(cpath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        free(cpath);
        return false;
    }

    BOOL res;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        res = RemoveDirectoryA(cpath);
    } else {
        res = DeleteFileA(cpath);
    }

    free(cpath);
    return res != 0;
}

static bool z_rm_recursive_internal(ZPathBuf* pb) {
    ZPathView current_pv = z_pathbuf_as_view(pb);
    char* cpath = z_sv_to_cstr_alloc(current_pv);
    if (!cpath) return false;

    DWORD attrs = GetFileAttributesA(cpath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        free(cpath);
        return false;
    }

    if (!(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        BOOL res = DeleteFileA(cpath);
        free(cpath);
        return res != 0;
    }

    ZPathBuf search_pb;
    z_pathbuf_init_from(&search_pb, current_pv);
    z_pathbuf_join(&search_pb, Z_SV("*"));
    char* csearch = z_sv_to_cstr_alloc(z_pathbuf_as_view(&search_pb));
    z_pathbuf_destroy(&search_pb);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(csearch, &fd);
    free(csearch);

    if (hFind == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
             BOOL res = RemoveDirectoryA(cpath);
             free(cpath);
             return res != 0;
        }
        free(cpath);
        return false;
    }

    bool success = true;
    usize original_len = pb->len;

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
            continue;
        }

        if (!z_pathbuf_join(pb, z_sv_from_cstr(fd.cFileName))) {
            success = false;
            break;
        }

        if (!z_rm_recursive_internal(pb)) {
            success = false;
        }

        pb->len = original_len;
        if (!success) break;
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);

    if (success) {
        success = (RemoveDirectoryA(cpath) != 0);
    }

    free(cpath);
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

    DWORD attrs = GetFileAttributesA(cpath);
    free(cpath);
    return (attrs != INVALID_FILE_ATTRIBUTES);
}

ZFileType z_get_file_type(ZPathView path) {
    if (path.len == 0) return Z_FILE_NOT_FOUND;
    char* cpath = z_sv_to_cstr_alloc(path);
    if (!cpath) return Z_FILE_NOT_FOUND;

    DWORD attrs = GetFileAttributesA(cpath);
    free(cpath);
    if (attrs == INVALID_FILE_ATTRIBUTES) return Z_FILE_NOT_FOUND;
    if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) return Z_FILE_SYMLINK;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) return Z_FILE_DIR;
    return Z_FILE_REGULAR;
}

bool z_set_executable(ZPathView path, bool enabled) {
    (void)path;
    (void)enabled;
    // On Windows, execution depends on file extension, not file attributes.
    return true;
}

#endif // Z_PLATFORM_IS_WINDOWS
