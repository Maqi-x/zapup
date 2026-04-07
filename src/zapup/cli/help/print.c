#include <zapup/cli/help/print.h>
#include <zapup/cli/ansi.h>

#include <stdio.h>

void z_cli_print_help(const ZHelpInfo* info, FILE* out) {
    fprintf(out, BOLD Z_SV_FMT " help\n" RESET, Z_SV_FARG(info->name));

    if (info->desc.len > 0) {
        fputc(' ', out);
        z_sv_print(info->desc, out);
        fputc('\n', out);
    }
    fputc('\n', out);

    fputs(BOLD "global flags:\n" RESET, out);
    z_cli_print_help_flags(info->global_flags, info->global_flags_count, out);
    fputc('\n', out);

    fputs(BOLD "build flags:\n" RESET, out);
    fputs("those flags works for commands that builds zap", out);
    fputs("for now: " BOLD "install and sync" RESET, out);
    z_cli_print_help_flags(info->global_flags, info->global_flags_count, out);
    fputc('\n', out);

    if (info->cmds != NULL) {
        for (const ZHelpCommand* cmd = info->cmds; cmd < info->cmds + info->command_count; ++cmd) {
            z_cli_print_help_cmd(cmd, out);
            fputc('\n', out);
        }
    }

    if (info->footer.len > 0) {
        fputc(' ', out);
        z_sv_print(info->footer, out);
        fputc('\n', out);
    }
}

static const usize OFFSET = 2;

void z_cli_print_help_flags(const ZHelpFlag* flags, usize count, FILE* out) {
    if (flags == NULL || count == 0) {
        return;
    }

    usize max_name_len = 0;
    for (usize i = 0; i < count; ++i) {
        if (flags[i].name.len > max_name_len) {
            max_name_len = flags[i].name.len;
        }
    }

    for (usize i = 0; i < count; ++i) {
        usize pad = max_name_len + OFFSET - flags[i].name.len;

        fputs("  ", out);
        z_sv_print(flags[i].name, out);

        for (usize j = 0; j < pad; ++j) {
            fputc(' ', out);
        }
        z_sv_print(flags[i].desc, out);
        fputc('\n', out);
    }
}

void z_cli_print_help_cmd(const ZHelpCommand* cmd, FILE* out) {
    fprintf(out, BOLD "command " Z_SV_FMT ":\n" RESET, Z_SV_FARG(cmd->name));

    if (cmd->desc.len > 0) {
        fputc(' ', out);
        z_sv_print(cmd->desc, out);
        fputc('\n', out);
    }

    if (cmd->flags != NULL) {
        usize count = 0;
        for (const ZHelpFlag* flag = cmd->flags; !z_sv_is_null(flag->name); ++flag) {
            ++count;
        }

        z_cli_print_help_flags(cmd->flags, count, out);
    }
}
