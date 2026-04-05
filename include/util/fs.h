#pragma once

#include <util/pathview.h>

bool z_mkdir(ZPathView path);
bool z_mkdir_all(ZPathView path);
bool z_mkfile(ZPathView path);

bool Z_mkdir_if_not_exists();
bool z_mkfile_if_not_exists();

bool z_touch(ZPathView path);
bool z_rm(ZPathView path);
bool z_rm_recursive(ZPathView path);

typedef enum ZFileType {
    Z_FILE_REGULAR,
    Z_FILE_DIR,
    Z_FILE_SYMLINK,
    Z_FILE_OTHER,
    Z_FILE_NOT_FOUND,
} ZFileType;

bool z_file_exists(ZPathView path);
ZFileType z_get_file_type(ZPathView path);

bool z_set_executable(ZPathView path, bool enabled);
