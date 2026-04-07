#pragma once

#include <zapup/cli/help/settings.h>

void z_cli_print_help(const ZHelpInfo* info, FILE* out);
void z_cli_print_help_cmd(const ZHelpCommand* cmd, FILE* out);
void z_cli_print_help_flags(const ZHelpFlag* flags, usize count, FILE* out);
