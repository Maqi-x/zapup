#include <zapup/index.h>
#include <yyjson.h>
#include <stdlib.h>
#include <string.h>

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
    free(idx->entries);
    z_version_index_init(idx);
}

void z_version_index_add(ZVersionIndex* idx, ZResolvableZapVersion version, ZPathView path) {
    if (idx->len >= idx->cap) {
        idx->cap = idx->cap == 0 ? 8 : idx->cap * 2;
        idx->entries = realloc(idx->entries, idx->cap * sizeof(ZVersionIndexEntry));
    }
    idx->entries[idx->len++] = (ZVersionIndexEntry){
        .version = version,
        .path = path,
    };
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
        yyjson_val* v_commit = yyjson_obj_get(val, "commit");
        yyjson_val* v_path = yyjson_obj_get(val, "path");

        if (v_branch && v_commit && v_path) {
            ZResolvableZapVersion ver = {
                .branch = z_sv_from_data_and_len(yyjson_get_str(v_branch), yyjson_get_len(v_branch)),
                .commit = z_sv_from_data_and_len(yyjson_get_str(v_commit), yyjson_get_len(v_commit)),
            };
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
        yyjson_mut_obj_add_strn(doc, obj, "branch", entry->version.branch.data, entry->version.branch.len);
        yyjson_mut_obj_add_strn(doc, obj, "commit", entry->version.commit.data, entry->version.commit.len);
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
