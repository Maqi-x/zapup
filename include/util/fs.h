#pragma once

#include <util/pathview.h>

bool z_mkdir(ZPathView path);
bool z_mkdir_all(ZPathView path);
bool z_mkfile(ZPathView path);
bool z_touch(ZPathView path);

bool z_set_executable(ZPathView path, bool enabled);
