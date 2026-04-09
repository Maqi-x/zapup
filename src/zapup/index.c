#include <zapup/index.h>
#include <util/fs.h>

#include <yyjson.h>
#include <stdlib.h>
#include <string.h>

ZapVersion z_version_index_entry_version(ZVersionIndexEntry* entry) {
    ZStringView r = entry->revspec.len == 0 ? Z_SV_NULL : z_strbuf_view(&entry->revspec);
    ZapVersion v = (ZapVersion) {
        .branch = entry->branch.len == 0 ? Z_SV_NULL : z_strbuf_view(&entry->branch),
        .ref_kind = Z_REF_REVSPEC,
        .revspec = r,
        .build = entry->build,
    };

    if (z_sv_eql(r, Z_SV("HEAD")) || z_sv_eql(r, Z_SV("latest"))) {
        v.ref_kind = Z_REF_HEAD;
        v.revspec = Z_SV_NULL;
    }

    return v;
}

void z_version_index_init(ZVersionIndex* idx) {
    idx->entries = NULL;
    idx->len = 0;
    idx->cap = 0;
    idx->_ctx = NULL;
}

void z_version_index_free(ZVersionIndex* idx) {
    if (idx->_ctx) {
        yyjson_doc_free((yyjson_doc*)idx->_ctx);
    }
    for (usize i = 0; i < idx->len; i++) {
        z_strbuf_destroy(&idx->entries[i].branch);
        z_strbuf_destroy(&idx->entries[i].revspec);
        z_strbuf_destroy(&idx->entries[i].path);
    }
    free(idx->entries);
    z_version_index_init(idx);
}

void z_version_index_add(ZVersionIndex* idx, ZapVersion version, ZPathView path) {
    if (idx->len >= idx->cap) {
        idx->cap = idx->cap == 0 ? 8 : idx->cap * 2;
        idx->entries = realloc(idx->entries, idx->cap * sizeof(ZVersionIndexEntry));
    }

    ZVersionIndexEntry* entry = &idx->entries[idx->len++];
    z_strbuf_init_from(&entry->branch, version.branch);
    z_strbuf_init_from(&entry->revspec, version.revspec);
    entry->build = version.build;
    z_strbuf_init_from(&entry->path, path);
}

ZVersionIndexEntry* z_version_index_find_by_version(ZVersionIndex* idx, ZapVersion version) {
    for (usize i = 0; i < idx->len; i++) {
        ZVersionIndexEntry* entry = &idx->entries[i];

        ZStringView entry_branch = z_strbuf_view(&entry->branch);
        ZStringView entry_revspec = entry->revspec.len == 0 ? Z_SV_NULL : z_strbuf_view(&entry->revspec);

        ZRefKind entry_ref_kind = Z_REF_REVSPEC;
        if (z_sv_is_null(entry_revspec) || entry_revspec.len == 0 ||
            z_sv_eql(entry_revspec, Z_SV("HEAD")) || z_sv_eql(entry_revspec, Z_SV("latest"))) {
            entry_ref_kind = Z_REF_HEAD;
        }

        bool ref_equal = false;
        if (entry_ref_kind == version.ref_kind) {
            if (entry_ref_kind == Z_REF_REVSPEC) {
                ref_equal = z_sv_eql(entry_revspec, version.revspec);
            } else {
                ref_equal = true;
            }
        } else {
            ref_equal = false;
        }

        if (z_sv_eql(entry_branch, version.branch) && ref_equal && entry->build == version.build) {
            return entry;
        }
    }
    return NULL;
}

ZVersionIndexEntry* z_version_index_find_by_path(ZVersionIndex* idx, ZPathView path) {
    for (usize i = 0; i < idx->len; i++) {
        ZVersionIndexEntry* entry = &idx->entries[i];
        if (z_sv_eql(z_strbuf_view(&entry->path), path)) {
            return entry;
        }
    }
    return NULL;
}

void z_version_index_remove_at(ZVersionIndex* idx, usize i) {
    if (i >= idx->len) return;
    ZVersionIndexEntry* entry = &idx->entries[i];
    z_strbuf_destroy(&entry->branch);
    z_strbuf_destroy(&entry->revspec);
    z_strbuf_destroy(&entry->path);
    if (i < idx->len - 1) {
        memmove(&idx->entries[i], &idx->entries[i + 1], (idx->len - i - 1) * sizeof(ZVersionIndexEntry));
    }
    idx->len--;
}

bool z_version_index_remove_by_version(ZVersionIndex* idx, ZapVersion version) {
    for (usize i = 0; i < idx->len; i++) {
        ZVersionIndexEntry* entry = &idx->entries[i];

        ZStringView entry_branch = z_strbuf_view(&entry->branch);
        ZStringView entry_revspec = entry->revspec.len == 0 ? Z_SV_NULL : z_strbuf_view(&entry->revspec);

        ZRefKind entry_ref_kind = Z_REF_REVSPEC;
        if (z_sv_is_null(entry_revspec) || entry_revspec.len == 0 ||
            z_sv_eql(entry_revspec, Z_SV("HEAD")) || z_sv_eql(entry_revspec, Z_SV("latest"))) {
            entry_ref_kind = Z_REF_HEAD;
        }

        bool ref_equal = false;
        if (entry_ref_kind == version.ref_kind) {
            if (entry_ref_kind == Z_REF_REVSPEC) {
                ref_equal = z_sv_eql(entry_revspec, version.revspec);
            } else {
                ref_equal = true;
            }
        }

        if (z_sv_eql(entry_branch, version.branch) && ref_equal && entry->build == version.build) {
            z_version_index_remove_at(idx, i);
            return true;
        }
    }
    return false;
}

bool z_version_index_remove_by_path(ZVersionIndex* idx, ZPathView path) {
    for (usize i = 0; i < idx->len; i++) {
        ZVersionIndexEntry* entry = &idx->entries[i];
        if (z_sv_eql(z_strbuf_view(&entry->path), path)) {
            z_version_index_remove_at(idx, i);
            return true;
        }
    }
    return false;
}

void z_version_index_from_json(ZVersionIndex* idx, ZStringView json) {
    z_version_index_free(idx);

    yyjson_read_flag flg = YYJSON_READ_NOFLAG;
    yyjson_read_err err;
    yyjson_doc* doc = yyjson_read_opts((char*)json.data, json.len, flg, NULL, &err);
    if (!doc) return;

    idx->_ctx = doc;
    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_arr(root)) return;

    yyjson_val* val;
    yyjson_arr_iter iter;
    yyjson_arr_iter_init(root, &iter);
    while ((val = yyjson_arr_iter_next(&iter))) {
        yyjson_val* v_branch = yyjson_obj_get(val, "branch");
        yyjson_val* v_revspec = yyjson_obj_get(val, "revspec");
        yyjson_val* v_build = yyjson_obj_get(val, "build");
        yyjson_val* v_path = yyjson_obj_get(val, "path");

        if (v_branch && v_revspec && v_path) {
            ZBuildType build = Z_BUILD_RELEASE;
            if (v_build) {
                const char* bstr = yyjson_get_str(v_build);
                if (bstr && strcmp(bstr, "debug") == 0) {
                    build = Z_BUILD_DEBUG;
                }
            }

            ZStringView branch_sv = z_sv_from_data_and_len(yyjson_get_str(v_branch), yyjson_get_len(v_branch));
            ZStringView revspec_sv = z_sv_from_data_and_len(yyjson_get_str(v_revspec), yyjson_get_len(v_revspec));

            ZapVersion ver;
            ver.branch = branch_sv;
            ver.build = build;

            if (z_sv_is_null(revspec_sv) || revspec_sv.len == 0 ||
                z_sv_eql(revspec_sv, Z_SV("HEAD")) || z_sv_eql(revspec_sv, Z_SV("latest"))) {
                ver.ref_kind = Z_REF_HEAD;
                ver.revspec = Z_SV_NULL;
            } else {
                ver.ref_kind = Z_REF_REVSPEC;
                ver.revspec = revspec_sv;
            }

            ZPathView path = z_sv_from_data_and_len(yyjson_get_str(v_path), yyjson_get_len(v_path));
            z_version_index_add(idx, ver, path);
        }
    }
}

bool z_version_index_to_json(ZVersionIndex* idx, ZStringBuf* out) {
    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val* root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);

    for (usize i = 0; i < idx->len; i++) {
        ZVersionIndexEntry* entry = &idx->entries[i];
        yyjson_mut_val* obj = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strn(doc, obj, "branch", entry->branch.data, entry->branch.len);
        yyjson_mut_obj_add_strn(doc, obj, "revspec", entry->revspec.data, entry->revspec.len);
        yyjson_mut_obj_add_str(doc, obj, "build", entry->build == Z_BUILD_DEBUG ? "debug" : "release");
        yyjson_mut_obj_add_strn(doc, obj, "path", entry->path.data, entry->path.len);
        yyjson_mut_arr_append(root, obj);
    }

    yyjson_write_err err;
    usize len;
    char* json = yyjson_mut_write_opts(doc, YYJSON_WRITE_PRETTY, NULL, &len, &err);

    bool ok = false;
    if (json) {
        ok = z_strbuf_append(out, z_sv_from_data_and_len(json, len));
        free(json);
    }

    yyjson_mut_doc_free(doc);
    return ok;
}

bool z_version_index_load(ZVersionIndex* idx, ZPathView path) {
    ZStringBuf content;
    z_strbuf_init(&content);
    if (!z_read_file(path, &content)) {
        z_strbuf_destroy(&content);
        return false;
    }

    z_version_index_from_json(idx, z_strbuf_view(&content));
    z_strbuf_destroy(&content);
    return true;
}

bool z_version_index_save(ZVersionIndex* idx, ZPathView path) {
    ZStringBuf json;
    z_strbuf_init(&json);
    if (!z_version_index_to_json(idx, &json)) {
        z_strbuf_destroy(&json);
        return false;
    }

    bool ok = z_write_file(path, z_strbuf_view(&json));
    z_strbuf_destroy(&json);
    return ok;
}
