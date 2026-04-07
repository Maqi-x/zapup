#include <zapup/config.h>
#include <util/fs.h>

#include <yyjson.h>
#include <stdlib.h>
#include <string.h>

void z_config_init(ZConfig* cfg) {
    cfg->toolchain.active_version = Z_ZAP_VERSION_NULL;
    cfg->build.cc = Z_SV("cc");
    cfg->build.cxx = Z_SV("c++");
    cfg->_ctx = NULL;
}

void z_config_free(ZConfig* cfg) {
    if (cfg->_ctx) {
        yyjson_doc_free((yyjson_doc*)cfg->_ctx);
    }
    z_config_init(cfg);
}

void z_config_from_json(ZConfig* cfg, ZStringView json) {
    z_config_free(cfg);

    yyjson_read_flag flg = YYJSON_READ_NOFLAG;
    yyjson_read_err err;
    yyjson_doc* doc = yyjson_read_opts((char*)json.data, json.len, flg, NULL, &err);
    if (!doc) return;

    cfg->_ctx = doc;
    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_obj(root)) return;

    yyjson_val* v_toolchain = yyjson_obj_get(root, "toolchain");
    if (yyjson_is_obj(v_toolchain)) {
        yyjson_val* v_active = yyjson_obj_get(v_toolchain, "active_version");
        if (yyjson_is_obj(v_active)) {
            yyjson_val* v_branch = yyjson_obj_get(v_active, "branch");
            if (yyjson_is_str(v_branch)) {
                cfg->toolchain.active_version.branch = z_sv_from_data_and_len(yyjson_get_str(v_branch), yyjson_get_len(v_branch));
            }

            yyjson_val* v_commit = yyjson_obj_get(v_active, "commit");
            if (yyjson_is_str(v_commit)) {
                cfg->toolchain.active_version.commit = z_sv_from_data_and_len(yyjson_get_str(v_commit), yyjson_get_len(v_commit));
            }

            yyjson_val* v_build = yyjson_obj_get(v_active, "build");
            if (yyjson_is_str(v_build)) {
                const char* bstr = yyjson_get_str(v_build);
                if (bstr && strcmp(bstr, "debug") == 0) {
                    cfg->toolchain.active_version.build = Z_BUILD_DEBUG;
                } else if (bstr && strcmp(bstr, "release") == 0) {
                    cfg->toolchain.active_version.build = Z_BUILD_RELEASE;
                }
            }
        }
    }

    yyjson_val* v_build = yyjson_obj_get(root, "build");
    if (yyjson_is_obj(v_build)) {
        yyjson_val* v_cc = yyjson_obj_get(v_build, "cc");
        if (yyjson_is_str(v_cc)) {
            cfg->build.cc = z_sv_from_data_and_len(yyjson_get_str(v_cc), yyjson_get_len(v_cc));
        }

        yyjson_val* v_cxx = yyjson_obj_get(v_build, "cxx");
        if (yyjson_is_str(v_cxx)) {
            cfg->build.cxx = z_sv_from_data_and_len(yyjson_get_str(v_cxx), yyjson_get_len(v_cxx));
        }
    }
}

bool z_config_to_json(const ZConfig* cfg, ZStringBuf* out) {
    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    // Toolchain
    yyjson_mut_val* toolchain = yyjson_mut_obj(doc);
    yyjson_mut_obj_add_val(doc, root, "toolchain", toolchain);

    if (!z_zap_ver_is_null(cfg->toolchain.active_version)) {
        yyjson_mut_val* active = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_val(doc, toolchain, "active_version", active);
        
        yyjson_mut_obj_add_strn(doc, active, "branch", 
            cfg->toolchain.active_version.branch.data, 
            cfg->toolchain.active_version.branch.len);
        yyjson_mut_obj_add_strn(doc, active, "commit", 
            cfg->toolchain.active_version.commit.data, 
            cfg->toolchain.active_version.commit.len);
        yyjson_mut_obj_add_str(doc, active, "build", 
            cfg->toolchain.active_version.build == Z_BUILD_DEBUG ? "debug" : "release");
    }

    // Build
    yyjson_mut_val* build = yyjson_mut_obj(doc);
    yyjson_mut_obj_add_val(doc, root, "build", build);

    if (!z_sv_is_null(cfg->build.cc)) {
        yyjson_mut_obj_add_strn(doc, build, "cc", cfg->build.cc.data, cfg->build.cc.len);
    }
    if (!z_sv_is_null(cfg->build.cxx)) {
        yyjson_mut_obj_add_strn(doc, build, "cxx", cfg->build.cxx.data, cfg->build.cxx.len);
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

bool z_config_load(ZConfig* cfg, ZPathView path) {
    ZStringBuf content;
    z_strbuf_init(&content);
    if (!z_read_file(path, &content)) {
        z_strbuf_destroy(&content);
        return false;
    }

    z_config_from_json(cfg, z_strbuf_view(&content));
    z_strbuf_destroy(&content);
    return true;
}

bool z_config_save(const ZConfig* cfg, ZPathView path) {
    ZStringBuf json;
    z_strbuf_init(&json);
    if (!z_config_to_json(cfg, &json)) {
        z_strbuf_destroy(&json);
        return false;
    }

    bool ok = z_write_file(path, z_strbuf_view(&json));
    z_strbuf_destroy(&json);
    return ok;
}
