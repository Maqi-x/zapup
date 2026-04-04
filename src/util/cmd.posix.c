#include <defs/platform.h>

#if Z_PLATFORM_IS_POSIX

#include <util/cmd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

typedef struct {
    int stdout_pipe[2];
    int stderr_pipe[2];
} ZPipes;

static char* z_sv_to_cstr(ZStringView sv) {
    if (z_sv_is_null(sv)) return NULL;
    char* cstr = malloc(sv.len + 1);
    if (cstr == NULL) return NULL;
    memcpy(cstr, sv.data, sv.len);
    cstr[sv.len] = '\0';
    return cstr;
}

static bool z_cmd_setup_pipes(const ZCommand* cmd, ZPipes* pipes) {
    pipes->stdout_pipe[0] = pipes->stdout_pipe[1] = -1;
    pipes->stderr_pipe[0] = pipes->stderr_pipe[1] = -1;

    if (cmd->capture_stdout != NULL) {
        if (pipe(pipes->stdout_pipe) == -1) return false;
    }
    if (cmd->capture_stderr != NULL) {
        if (pipe(pipes->stderr_pipe) == -1) {
            if (pipes->stdout_pipe[0] != -1) {
                close(pipes->stdout_pipe[0]);
                close(pipes->stdout_pipe[1]);
            }
            return false;
        }
    }
    return true;
}

static void z_cmd_child_setup(const ZCommand* cmd, const ZPipes* pipes) {
    if (!z_sv_is_null(cmd->cwd) && cmd->cwd.len > 0) {
        char* cwd_cstr = z_sv_to_cstr(cmd->cwd);
        if (cwd_cstr) {
            if (chdir(cwd_cstr) == -1) {
                perror("chdir");
                _exit(127);
            }
            free(cwd_cstr);
        }
    }

    if (cmd->capture_stdout != NULL) {
        dup2(pipes->stdout_pipe[1], STDOUT_FILENO);
        close(pipes->stdout_pipe[0]);
        close(pipes->stdout_pipe[1]);
    }
    if (cmd->capture_stderr != NULL) {
        dup2(pipes->stderr_pipe[1], STDERR_FILENO);
        close(pipes->stderr_pipe[0]);
        close(pipes->stderr_pipe[1]);
    }

    char** argv = malloc((cmd->argv.count + 1) * sizeof(char*));
    if (argv) {
        for (usize i = 0; i < cmd->argv.count; i++) {
            argv[i] = z_sv_to_cstr(cmd->argv.data[i]);
        }
        argv[cmd->argv.count] = NULL;
    }

    if (cmd->envp.count > 0) {
        char** envp = malloc((cmd->envp.count + 1) * sizeof(char*));
        if (envp) {
            for (usize i = 0; i < cmd->envp.count; i++) {
                envp[i] = z_sv_to_cstr(cmd->envp.data[i]);
            }
            envp[cmd->envp.count] = NULL;
            extern char **environ;
            environ = envp;
        }
    }

    if (argv && argv[0]) {
        execvp(argv[0], argv);
    }
    _exit(127);
}

static void z_cmd_capture_output(const ZCommand* cmd, ZPipes* pipes) {
    if (cmd->capture_stdout != NULL) close(pipes->stdout_pipe[1]);
    if (cmd->capture_stderr != NULL) close(pipes->stderr_pipe[1]);

    struct pollfd pfd[2];
    int n_pfds = 0;
    if (cmd->capture_stdout != NULL) {
        pfd[n_pfds].fd = pipes->stdout_pipe[0];
        pfd[n_pfds].events = POLLIN;
        n_pfds++;
    }
    if (cmd->capture_stderr != NULL) {
        pfd[n_pfds].fd = pipes->stderr_pipe[0];
        pfd[n_pfds].events = POLLIN;
        n_pfds++;
    }

    while (n_pfds > 0) {
        if (poll(pfd, n_pfds, -1) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        for (int i = 0; i < n_pfds; i++) {
            if (pfd[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                char buf[4096];
                ssize_t n = read(pfd[i].fd, buf, sizeof(buf));
                if (n > 0) {
                    if (cmd->capture_stdout && pfd[i].fd == pipes->stdout_pipe[0]) {
                        z_strbuf_append(cmd->capture_stdout, z_sv_from_data_and_len(buf, (usize)n));
                    } else if (cmd->capture_stderr && pfd[i].fd == pipes->stderr_pipe[0]) {
                        z_strbuf_append(cmd->capture_stderr, z_sv_from_data_and_len(buf, (usize)n));
                    }
                } else if (n == 0 || (n == -1 && errno != EINTR && errno != EAGAIN)) {
                    close(pfd[i].fd);
                    pfd[i] = pfd[n_pfds - 1];
                    n_pfds--;
                    i--;
                }
            } else if (pfd[i].revents & POLLNVAL) {
                close(pfd[i].fd);
                pfd[i] = pfd[n_pfds - 1];
                n_pfds--;
                i--;
            }
        }
    }
}

ZCmdRunResult z_cmd_run(const ZCommand* cmd) {
    ZCmdRunResult result = { .status = Z_CMD_SPAWN_ERROR, .exit_code = -1 };
    ZPipes pipes;

    if (!z_cmd_setup_pipes(cmd, &pipes)) return result;

    pid_t pid = fork();
    if (pid == -1) {
        if (pipes.stdout_pipe[0] != -1) { close(pipes.stdout_pipe[0]); close(pipes.stdout_pipe[1]); }
        if (pipes.stderr_pipe[0] != -1) { close(pipes.stderr_pipe[0]); close(pipes.stderr_pipe[1]); }
        return result;
    }

    if (pid == 0) {
        z_cmd_child_setup(cmd, &pipes);
    }

    z_cmd_capture_output(cmd, &pipes);

    int status;
    if (waitpid(pid, &status, 0) != -1) {
        result.status = Z_CMD_OK;
        if (WIFEXITED(status)) {
            result.exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            result.exit_code = 128 + WTERMSIG(status);
        }
    }

    return result;
}

#endif
