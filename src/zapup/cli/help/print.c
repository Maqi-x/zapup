#include <zapup/cli/help/print.h>
#include <zapup/cli/ansi.h>

#include <stdio.h>

static const usize OFFSET = 2;

static usize z_help_max_len(const ZHelpFlag* flags, usize count) {
    usize max_len = 0;
    for (usize i = 0; i < count; ++i) {
        if (flags[i].name.len > max_len) {
            max_len = flags[i].name.len;
        }
    }
    return max_len;
}

static bool z_cli_cmd_has_build_flags(ZCliCommand cmd) {
    return cmd == Z_CLI_CMD_INSTALL || cmd == Z_CLI_CMD_SYNC;
}

static void z_cli_print_usage_entries(ZHelpUsage usage, FILE* out) {
    if (usage.entries == NULL || usage.count == 0) {
        return;
    }

    usize max_name_len = 0;
    for (usize i = 0; i < usage.count; ++i) {
        if (usage.entries[i].name.len > max_name_len) {
            max_name_len = usage.entries[i].name.len;
        }
    }

    fputs(BOLD "Arguments:\n" RESET, out);
    for (usize i = 0; i < usage.count; ++i) {
        usize pad = max_name_len + OFFSET - usage.entries[i].name.len;
        fputs("  ", out);
        z_sv_print(usage.entries[i].name, out);
        for (usize j = 0; j < pad; ++j) {
            fputc(' ', out);
        }
        z_sv_print(usage.entries[i].desc, out);
        fputs(usage.entries[i].optional ? " (optional)" : " (required)", out);
        fputc('\n', out);
    }
}

void z_cli_print_help(const ZHelpInfo* info, FILE* out) {
    fprintf(out, BOLD Z_SV_FMT RESET " - help\n", Z_SV_FARG(info->name));

    if (info->desc.len > 0) {
        z_sv_print(info->desc, out);
        fputc('\n', out);
    }
    fputc('\n', out);

    fprintf(out, BOLD "Usage:\n" RESET "  " Z_SV_FMT " <command> [arguments] [flags]\n\n", Z_SV_FARG(info->name));

    fputs(BOLD "Commands:\n" RESET, out);
    if (info->cmds != NULL && info->command_count > 0) {
        usize max_name_len = 0;
        for (const ZHelpCommand* cmd = info->cmds; cmd < info->cmds + info->command_count; ++cmd) {
            if (cmd->name.len > max_name_len) {
                max_name_len = cmd->name.len;
            }
        }

        for (const ZHelpCommand* cmd = info->cmds; cmd < info->cmds + info->command_count; ++cmd) {
            usize pad = max_name_len + OFFSET - cmd->name.len;
            fputs("  ", out);
            z_sv_print(cmd->name, out);
            for (usize j = 0; j < pad; ++j) {
                fputc(' ', out);
            }
            z_sv_print(cmd->desc, out);
            fputc('\n', out);
        }
    }
    fprintf(out, "\n  Run `" Z_SV_FMT " help <command>` for detailed command usage.\n\n", Z_SV_FARG(info->name));

    fputs(BOLD "Global flags:\n" RESET, out);
    z_cli_print_help_flags(info->global_flags.flags, info->global_flags.count, out);
    fputc('\n', out);

    fputs(BOLD "Build flags:\n" RESET, out);
    fputs("  Available for: install, sync\n", out);
    z_cli_print_help_flags(info->build_flags.flags, info->build_flags.count, out);
    fputc('\n', out);

    if (info->footer.len > 0) {
        z_sv_print(info->footer, out);
        fputc('\n', out);
    }
}

static void z_cli_print_help_flags_with_max_len(const ZHelpFlag* flags, usize count, usize max_name_len, FILE* out) {
    if (flags == NULL || count == 0) {
        return;
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

void z_cli_print_help_flags(const ZHelpFlag* flags, usize count, FILE* out) {
    if (flags == NULL || count == 0) {
        return;
    }

    usize max_name_len = z_help_max_len(flags, count);
    z_cli_print_help_flags_with_max_len(flags, count, max_name_len, out);
}

void z_cli_print_help_cmd_usage(const ZHelpInfo* info, const ZHelpCommand* cmd, FILE* out) {
    if (cmd == NULL) {
        return;
    }

    fputs("  ", out);
    if (info != NULL && info->name.len > 0) {
        z_sv_print(info->name, out);
        fputc(' ', out);
    }
    z_sv_print(cmd->name, out);

    for (usize i = 0; i < cmd->usage.count; ++i) {
        const ZHelpUsageEntry* entry = &cmd->usage.entries[i];
        fputc(' ', out);
        if (entry->optional) fputc('[', out);
        fputc('<', out);
        z_sv_print(entry->name, out);
        fputc('>', out);
        if (entry->optional) fputc(']', out);
    }

    if ((cmd->flags.flags != NULL && cmd->flags.count > 0) || (info != NULL && z_cli_cmd_has_build_flags(cmd->cmd) && info->build_flags.count > 0)) {
        fputs(" [flags]", out);
    }
    fputc('\n', out);
}

void z_cli_print_help_cmd(const ZHelpInfo* info, const ZHelpCommand* cmd, FILE* out) {
    fprintf(out, BOLD "Command:\n" RESET "  " Z_SV_FMT "\n", Z_SV_FARG(cmd->name));

    if (cmd->desc.len > 0) {
        fputs(BOLD "Description:\n" RESET "  ", out);
        z_sv_print(cmd->desc, out);
        fputc('\n', out);
    }
    fputc('\n', out);

    fputs(BOLD "Usage:\n" RESET, out);
    z_cli_print_help_cmd_usage(info, cmd, out);
    fputc('\n', out);

    z_cli_print_usage_entries(cmd->usage, out);

    if (cmd->usage.entries != NULL && cmd->usage.count > 0) {
        fputc('\n', out);
    }

    bool has_cmd_flags = (cmd->flags.flags != NULL && cmd->flags.count > 0);
    bool has_build_flags = (info != NULL && z_cli_cmd_has_build_flags(cmd->cmd) && info->build_flags.count > 0);

    if (has_cmd_flags || has_build_flags) {
        fputs(BOLD "Flags:\n" RESET, out);

        usize max_len = 0;
        if (has_cmd_flags) {
            max_len = z_help_max_len(cmd->flags.flags, cmd->flags.count);
        }
        if (has_build_flags) {
            usize build_max = z_help_max_len(info->build_flags.flags, info->build_flags.count);
            if (build_max > max_len) max_len = build_max;
        }

        if (has_cmd_flags) {
            z_cli_print_help_flags_with_max_len(cmd->flags.flags, cmd->flags.count, max_len, out);
        }
        if (has_build_flags) {
            z_cli_print_help_flags_with_max_len(info->build_flags.flags, info->build_flags.count, max_len, out);
        }
    }
}
