#include <defs/platform.h>

#if Z_PLATFORM_IS_WINDOWS

#include <util/fs.h>
#include <util/pathview.h>
#include <util/pathbuf.h>

#include <windows.h>
#include <stdlib.h>

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

bool z_set_executable(ZPathView path, bool enabled) {
    (void)path;
    (void)enabled;
    // On Windows, execution depends on file extension, not file attributes.
    return true;
}

#endif // Z_PLATFORM_IS_WINDOWS
