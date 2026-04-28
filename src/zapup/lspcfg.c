#include <zapup/lspcfg.h>
#include <util/fs.h>
#include <yyjson.h>

#include <stdlib.h>
#include <string.h>

void z_lspcfg_from_json(ZLspConfig* cfg, ZStringView json) {
    cfg->targets = Z_LSP_UNKNOWN;
    cfg->nvim_mode = Z_NVIM_LSP_UNKNOWN;

    if (json.len == 0) return;

    yyjson_doc* doc = yyjson_read(json.data, json.len, 0);
    if (!doc) return;

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (yyjson_is_obj(root)) {
        yyjson_val* v_targets = yyjson_obj_get(root, "targets");
        if (yyjson_is_arr(v_targets)) {
            yyjson_val* val;
            usize idx, max;
            yyjson_arr_foreach(v_targets, idx, max, val) {
                if (yyjson_is_str(val)) {
                    ZStringView sv = z_sv_from_data_and_len(yyjson_get_str(val), yyjson_get_len(val));
                    cfg->targets |= z_parse_zap_lsp_target(sv);
                }
            }
        }

        yyjson_val* v_nvim_mode = yyjson_obj_get(root, "nvim-mode");
        if (yyjson_is_arr(v_nvim_mode)) {
            yyjson_val* val;
            usize idx, max;
            yyjson_arr_foreach(v_nvim_mode, idx, max, val) {
                if (yyjson_is_str(val)) {
                    ZStringView sv = z_sv_from_data_and_len(yyjson_get_str(val), yyjson_get_len(val));
                    cfg->nvim_mode |= z_parse_nvim_lsp_mode(sv);
                }
            }
        }
    }

    yyjson_doc_free(doc);
}

bool z_lspcfg_to_json(const ZLspConfig* cfg, ZStringBuf* out) {
    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_val* v_targets = yyjson_mut_arr(doc);
    if (cfg->targets & Z_LSP_NVIM) yyjson_mut_arr_add_str(doc, v_targets, "nvim");
    if (cfg->targets & Z_LSP_VSC) yyjson_mut_arr_add_str(doc, v_targets, "vsc");
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "targets"), v_targets);

    yyjson_mut_val* v_nvim_mode = yyjson_mut_arr(doc);
    if (cfg->nvim_mode & Z_NVIM_LSP_INIT_LUA) yyjson_mut_arr_add_str(doc, v_nvim_mode, "init.lua");
    if (cfg->nvim_mode & Z_NVIM_LSP_LAZY) yyjson_mut_arr_add_str(doc, v_nvim_mode, "lazy");
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "nvim-mode"), v_nvim_mode);

    char* json = yyjson_mut_write(doc, 0, NULL);
    if (json) {
        bool ok = z_strbuf_append_cstr(out, json);
        free(json);
        yyjson_mut_doc_free(doc);
        return ok;
    }

    yyjson_mut_doc_free(doc);
    return false;
}

bool z_lspcfg_load(ZLspConfig* cfg, ZPathView path) {
    ZStringBuf buf;
    if (!z_strbuf_init(&buf)) return false;
    if (!z_read_file(path, &buf)) {
        z_strbuf_destroy(&buf);
        return false;
    }
    z_lspcfg_from_json(cfg, z_strbuf_view(&buf));
    z_strbuf_destroy(&buf);
    return true;
}

bool z_lspcfg_save(const ZLspConfig* cfg, ZPathView path) {
    ZStringBuf buf;
    if (!z_strbuf_init(&buf)) return false;
    if (!z_lspcfg_to_json(cfg, &buf)) {
        z_strbuf_destroy(&buf);
        return false;
    }
    bool ok = z_write_file(path, z_strbuf_view(&buf));
    z_strbuf_destroy(&buf);
    return ok;
}
