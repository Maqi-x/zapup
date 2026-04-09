#pragma once

#include <util/pathview.h>
#include <util/strbuf.h>

#include <zapup/zap/version.h>
#include <defs/int-types.h>

typedef struct ZVersionIndexEntry {
    ZStringBuf branch;
    ZStringBuf revspec;
    ZRefKind ref_kind;
    ZBuildType build;
    ZStringBuf path;
} ZVersionIndexEntry;

typedef struct ZVersionIndex {
    ZVersionIndexEntry* entries;
    usize len, cap;
    void* _ctx;
} ZVersionIndex;

void z_version_index_init(ZVersionIndex* idx);
void z_version_index_free(ZVersionIndex* idx);

void z_version_index_add(ZVersionIndex* idx, ZapVersion version, ZPathView path);

ZVersionIndexEntry* z_version_index_find_by_version(ZVersionIndex* idx, ZapVersion version);
ZVersionIndexEntry* z_version_index_find_by_path(ZVersionIndex* idx, ZPathView path);

ZapVersion z_version_index_entry_version(ZVersionIndexEntry* entry);

void z_version_index_remove_at(ZVersionIndex* idx, usize i);
bool z_version_index_remove_by_version(ZVersionIndex* idx, ZapVersion version);
bool z_version_index_remove_by_path(ZVersionIndex* idx, ZPathView path);

void z_version_index_from_json(ZVersionIndex* idx, ZStringView json);
bool z_version_index_to_json(ZVersionIndex* idx, ZStringBuf* out);

bool z_version_index_load(ZVersionIndex* idx, ZPathView path);
bool z_version_index_save(ZVersionIndex* idx, ZPathView path);
