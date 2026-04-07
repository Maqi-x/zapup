#include <zapup/app/app.h>
#include <zapup/output.h>

#include <util/fs.h>

int zapup_ensure_index_lock(ZapupApp* app) {
    ZPathView lockpath = z_pathbuf_as_view(&app->paths.indexlock);
    if (!z_lockfile_lock(&app->indexlock, lockpath)) {
        ZPathBuf content;
        z_pathbuf_init(&content);
        z_read_file(lockpath, &content);
        z_show_error("cannot acquire lock. lock file exists (pid: " Z_SV_FMT ")", Z_SV_FARG(content));
        z_pathbuf_destroy(&content);
        return 2;
    }
    return 0;
}

int zapup_get_version_dir_init(ZapupApp* app, ZResolvableZapVersion ver, ZPathBuf* out_path) {
    ZStringBuf version_formatted;
    z_strbuf_init(&version_formatted);
    z_format_zap_version(ver, &version_formatted);

    z_pathbuf_init_from(out_path, z_pathbuf_as_view(&app->paths.versions));
    z_pathbuf_join(out_path, z_strbuf_view(&version_formatted));
    z_pathbuf_sanitize(out_path);

    z_strbuf_destroy(&version_formatted);
    return 0;
}
