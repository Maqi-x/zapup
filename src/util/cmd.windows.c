#include <defs/platform.h>

#if Z_PLATFORM_IS_WINDOWS

#include <util/cmd.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    HANDLE stdout_read;
    HANDLE stdout_write;
    HANDLE stderr_read;
    HANDLE stderr_write;
} ZWinPipes;

static char* z_sv_to_cstr(ZStringView sv) {
    if (z_sv_is_null(sv)) return NULL;
    char* cstr = malloc(sv.len + 1);
    if (cstr == NULL) return NULL;
    memcpy(cstr, sv.data, sv.len);
    cstr[sv.len] = '\0';
    return cstr;
}

static bool z_cmd_setup_pipes(const ZCommand* cmd, ZWinPipes* pipes, STARTUPINFOA* si) {
    pipes->stdout_read = pipes->stdout_write = NULL;
    pipes->stderr_read = pipes->stderr_write = NULL;

    SECURITY_ATTRIBUTES sa = { .nLength = sizeof(SECURITY_ATTRIBUTES), .lpSecurityDescriptor = NULL, .bInheritHandle = TRUE };

    if (cmd->capture_stdout != NULL) {
        if (!CreatePipe(&pipes->stdout_read, &pipes->stdout_write, &sa, 0)) return false;
        if (!SetHandleInformation(pipes->stdout_read, HANDLE_FLAG_INHERIT, 0)) return false;
        si->hStdOutput = pipes->stdout_write;
        si->dwFlags |= STARTF_USESTDHANDLES;
    }

    if (cmd->capture_stderr != NULL) {
        if (!CreatePipe(&pipes->stderr_read, &pipes->stderr_write, &sa, 0)) return false;
        if (!SetHandleInformation(pipes->stderr_read, HANDLE_FLAG_INHERIT, 0)) return false;
        si->hStdError = pipes->stderr_write;
        si->dwFlags |= STARTF_USESTDHANDLES;
    }

    return true;
}

static char* z_cmd_build_command_line(const ZStringViewArray* argv) {
    usize total_len = 0;
    for (usize i = 0; i < argv->count; i++) {
        total_len += argv->data[i].len + 3; // +2 for quotes, +1 for space
    }

    char* cmd_line = malloc(total_len + 1);
    if (!cmd_line) return NULL;

    char* ptr = cmd_line;
    for (usize i = 0; i < argv->count; i++) {
        *ptr++ = '"';
        memcpy(ptr, argv->data[i].data, argv->data[i].len);
        ptr += argv->data[i].len;
        *ptr++ = '"';
        if (i < argv->count - 1) *ptr++ = ' ';
    }
    *ptr = '\0';
    return cmd_line;
}

static char* z_cmd_build_env_block(const ZStringViewArray* envp) {
    if (envp->count == 0) return NULL;

    usize total_len = 0;
    for (usize i = 0; i < envp->count; i++) {
        total_len += envp->data[i].len + 1;
    }
    total_len += 1; // Final null terminator

    char* env_block = malloc(total_len);
    if (!env_block) return NULL;

    char* ptr = env_block;
    for (usize i = 0; i < envp->count; i++) {
        memcpy(ptr, envp->data[i].data, envp->data[i].len);
        ptr += envp->data[i].len;
        *ptr++ = '\0';
    }
    *ptr = '\0';
    return env_block;
}

static void z_cmd_capture_output(const ZCommand* cmd, ZWinPipes* pipes) {
    if (pipes->stdout_write) CloseHandle(pipes->stdout_write);
    if (pipes->stderr_write) CloseHandle(pipes->stderr_write);

    char buf[4096];
    DWORD bytes_read;

    while (pipes->stdout_read || pipes->stderr_read) {
        if (pipes->stdout_read) {
            DWORD available = 0;
            if (PeekNamedPipe(pipes->stdout_read, NULL, 0, NULL, &available, NULL) && available > 0) {
                if (ReadFile(pipes->stdout_read, buf, sizeof(buf), &bytes_read, NULL) && bytes_read > 0) {
                    z_strbuf_append(cmd->capture_stdout, z_sv_from_data_and_len(buf, (usize)bytes_read));
                }
            } else if (!available) {
                 // Check if the other end is closed
                 if (!PeekNamedPipe(pipes->stdout_read, NULL, 0, NULL, &available, NULL) && GetLastError() == ERROR_BROKEN_PIPE) {
                     CloseHandle(pipes->stdout_read);
                     pipes->stdout_read = NULL;
                 }
            }
        }

        if (pipes->stderr_read) {
            DWORD available = 0;
            if (PeekNamedPipe(pipes->stderr_read, NULL, 0, NULL, &available, NULL) && available > 0) {
                if (ReadFile(pipes->stderr_read, buf, sizeof(buf), &bytes_read, NULL) && bytes_read > 0) {
                    z_strbuf_append(cmd->capture_stderr, z_sv_from_data_and_len(buf, (usize)bytes_read));
                }
            } else if (!available) {
                 if (!PeekNamedPipe(pipes->stderr_read, NULL, 0, NULL, &available, NULL) && GetLastError() == ERROR_BROKEN_PIPE) {
                     CloseHandle(pipes->stderr_read);
                     pipes->stderr_read = NULL;
                 }
            }
        }
        Sleep(10);
    }
}

ZCmdRunResult z_cmd_run(const ZCommand* cmd) {
    ZCmdRunResult result = { .status = Z_CMD_SPAWN_ERROR, .exit_code = -1 };
    ZWinPipes pipes = { 0 };
    STARTUPINFOA si = { .cb = sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };

    if (!z_cmd_setup_pipes(cmd, &pipes, &si)) return result;

    char* cmd_line = z_cmd_build_command_line(&cmd->argv);
    char* env_block = z_cmd_build_env_block(&cmd->envp);
    char* cwd_cstr = z_sv_to_cstr(cmd->cwd);

    if (CreateProcessA(NULL, cmd_line, NULL, NULL, TRUE, 0, env_block, cwd_cstr, &si, &pi)) {
        z_cmd_capture_output(cmd, &pipes);
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exit_code;
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            result.status = Z_CMD_OK;
            result.exit_code = (int)exit_code;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    free(cmd_line);
    free(env_block);
    free(cwd_cstr);
    if (pipes.stdout_read) CloseHandle(pipes.stdout_read);
    if (pipes.stderr_read) CloseHandle(pipes.stderr_read);

    return result;
}

#endif
