#include <defs/platform.h>

#if Z_PLATFORM_IS_POSIX

#include <util/cmd.h>

#include <unistd.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

typedef struct {
    int stdout_pipe[2];
    int stderr_pipe[2];
} ZPipes;

static void z_cmd_close_pipes(ZPipes* pipes) {
    if (pipes->stdout_pipe[0] != -1) { close(pipes->stdout_pipe[0]); close(pipes->stdout_pipe[1]); }
    if (pipes->stderr_pipe[0] != -1) { close(pipes->stderr_pipe[0]); close(pipes->stderr_pipe[1]); }
}

static bool z_cmd_setup_pipes(const ZCommand* cmd, ZPipes* pipes) {
    pipes->stdout_pipe[0] = pipes->stdout_pipe[1] = -1;
    pipes->stderr_pipe[0] = pipes->stderr_pipe[1] = -1;

    if (cmd->capture_stdout != NULL && pipe(pipes->stdout_pipe) == -1) return false;
    if (cmd->capture_stderr != NULL && pipe(pipes->stderr_pipe) == -1) {
        z_cmd_close_pipes(pipes);
        return false;
    }
    return true;
}

static void z_cmd_child_setup(const ZCommand* cmd, const ZPipes* pipes, int err_fd) {
    if (!z_sv_is_null(cmd->cwd) && cmd->cwd.len > 0) {
        char* cwd_cstr = z_sv_to_cstr_alloc(cmd->cwd);
        if (!cwd_cstr || chdir(cwd_cstr) == -1) {
            int err = errno;
            (void)write(err_fd, &err, sizeof(err));
            _exit(1);
        }
        free(cwd_cstr);
    }

    if (cmd->capture_stdout) { dup2(pipes->stdout_pipe[1], STDOUT_FILENO); }
    if (cmd->capture_stderr) { dup2(pipes->stderr_pipe[1], STDERR_FILENO); }
    if (cmd->capture_stdout || cmd->capture_stderr) {
        z_cmd_close_pipes((ZPipes*)pipes);
    }

    char** argv = malloc((cmd->argv.count + 1) * sizeof(char*));
    for (usize i = 0; argv && i < cmd->argv.count; i++) argv[i] = z_sv_to_cstr_alloc(cmd->argv.data[i]);
    if (argv) argv[cmd->argv.count] = NULL;

    if (!argv || !argv[0]) {
        int err = ENOMEM;
        (void)write(err_fd, &err, sizeof(err));
        _exit(1);
    }

    execvp(argv[0], argv);
    int err = errno;
    (void)write(err_fd, &err, sizeof(err));
    _exit(1);
}

static void z_cmd_capture_output(const ZCommand* cmd, ZPipes* pipes) {
    if (cmd->capture_stdout) close(pipes->stdout_pipe[1]);
    if (cmd->capture_stderr) close(pipes->stderr_pipe[1]);

    struct pollfd pfd[2];
    int n = 0;
    if (cmd->capture_stdout) { pfd[n].fd = pipes->stdout_pipe[0]; pfd[n++].events = POLLIN; }
    if (cmd->capture_stderr) { pfd[n].fd = pipes->stderr_pipe[0]; pfd[n++].events = POLLIN; }

    while (n > 0) {
        if (poll(pfd, n, -1) <= 0) { if (errno == EINTR) continue; break; }
        for (int i = 0; i < n; i++) {
            if (pfd[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                char buf[4096];
                ssize_t r = read(pfd[i].fd, buf, sizeof(buf));
                if (r > 0) {
                    ZStringBuf* b = (cmd->capture_stdout && pfd[i].fd == pipes->stdout_pipe[0]) ? cmd->capture_stdout : cmd->capture_stderr;
                    z_strbuf_append(b, z_sv_from_data_and_len(buf, (usize)r));
                } else if (r == 0 || (r == -1 && errno != EINTR && errno != EAGAIN)) {
                    close(pfd[i].fd); pfd[i] = pfd[--n]; i--;
                }
            }
        }
    }
}

ZCmdRunResult z_cmd_run(const ZCommand* cmd) {
    ZCmdRunResult res = { .status = Z_CMD_LAUNCH_ERROR, .exit_code = -1 };
    ZPipes pipes;
    int err_pipe[2];

    if (!z_cmd_setup_pipes(cmd, &pipes)) return res;
    if (pipe(err_pipe) == -1) { z_cmd_close_pipes(&pipes); return res; }
    fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC);

    pid_t pid = fork();
    if (pid == -1) {
        close(err_pipe[0]); close(err_pipe[1]);
        z_cmd_close_pipes(&pipes);
        return res;
    }

    if (pid == 0) {
        close(err_pipe[0]);
        z_cmd_child_setup(cmd, &pipes, err_pipe[1]);
        _exit(1);
    }

    close(err_pipe[1]);
    int child_errno = 0;
    if (read(err_pipe[0], &child_errno, sizeof(child_errno)) > 0) {
        switch (child_errno) {
        case ENOENT:  res.status = Z_CMD_NOT_FOUND;          break;
        case EACCES:  res.status = Z_CMD_PERMISSION_DENIED;  break;
        case ENOTDIR: res.status = Z_CMD_CHDIR_ERROR;        break;
        default:      res.status = Z_CMD_LAUNCH_ERROR;       break;
        }
        waitpid(pid, NULL, 0);
    } else {
        z_cmd_capture_output(cmd, &pipes);
        int status;
        if (waitpid(pid, &status, 0) != -1) {
            res.status = Z_CMD_OK;
            res.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : (WIFSIGNALED(status) ? 128 + WTERMSIG(status) : -1);
        } else res.status = Z_CMD_WAIT_ERROR;
    }
    close(err_pipe[0]);
    if (pipes.stdout_pipe[0] != -1) close(pipes.stdout_pipe[0]);
    if (pipes.stderr_pipe[0] != -1) close(pipes.stderr_pipe[0]);
    return res;
}

#endif
