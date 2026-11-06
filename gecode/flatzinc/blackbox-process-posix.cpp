/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Jip J. Dekker, 2026
 */
#include <gecode/flatzinc/blackbox-process.hh>
#include <gecode/flatzinc.hh>

#if defined(GECODE_HAS_POSIX_BLACKBOX_EXEC) && !defined(_WIN32)

#include <cerrno>
#include <cstdio>
#include <memory>
#include <spawn.h>
#include <fcntl.h>
#include <pthread.h>
#include <csignal>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>
#include <unistd.h>

extern char** environ;

namespace Gecode::FlatZinc {
    namespace {

        const size_t max_exec_response_size = 1024 * 1024;

        int
        set_cloexec(int fd) {
            int flags = fcntl(fd, F_GETFD);
            if (flags == -1) {
                return -1;
            }
            return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
        }

        int
        dup_cloexec(int fd, int min_fd) {
            int nfd;
#ifdef F_DUPFD_CLOEXEC
            nfd = fcntl(fd, F_DUPFD_CLOEXEC, min_fd);
            if (nfd != -1) {
                return nfd;
            }
            if (errno != EINVAL) {
                return -1;
            }
#endif
            nfd = fcntl(fd, F_DUPFD, min_fd);
            if (nfd == -1) {
                return -1;
            }
            if (set_cloexec(nfd) != 0) {
                int e = errno;
                ::close(nfd);
                errno = e;
                return -1;
            }
            return nfd;
        }

        int
        move_from_standard_fd(int fd) {
            if (fd > STDERR_FILENO) {
                return fd;
            }
            int nfd = dup_cloexec(fd, STDERR_FILENO + 1);
            if (nfd == -1) {
                return -1;
            }
            ::close(fd);
            return nfd;
        }

        class FileDescriptor {
        private:
            int fd;

        public:
            explicit FileDescriptor(int fd0 = -1) : fd(fd0) {}
            ~FileDescriptor() {
                reset();
            }

            int
            get() const {
                return fd;
            }
            int
            release() {
                int fd0 = fd;
                fd = -1;
                return fd0;
            }
            void
            reset(int fd0 = -1) {
                if (fd != -1) {
                    ::close(fd);
                }
                fd = fd0;
            }
        };

        int
        move_away_from_standard_fd(FileDescriptor& fd) {
            int old = fd.release();
            int nfd = move_from_standard_fd(old);
            if (nfd == -1) {
                fd.reset(old);
            } else {
                fd.reset(nfd);
            }
            return nfd;
        }

        class SpawnFileActions {
        private:
            posix_spawn_file_actions_t actions;
            bool initialized;

        public:
            SpawnFileActions() : initialized(false) {}
            ~SpawnFileActions() {
                if (initialized) {
                    posix_spawn_file_actions_destroy(&actions);
                }
            }

            int
            init() {
                int err = posix_spawn_file_actions_init(&actions);
                initialized = err == 0;
                return err;
            }
            posix_spawn_file_actions_t*
            get() {
                return &actions;
            }
        };

        class SpawnAttributes {
        private:
            posix_spawnattr_t attr;
            bool initialized;

        public:
            SpawnAttributes() : initialized(false) {}
            ~SpawnAttributes() {
                if (initialized) {
                    posix_spawnattr_destroy(&attr);
                }
            }

            int
            init() {
                int err = posix_spawnattr_init(&attr);
                initialized = err == 0;
                return err;
            }
            posix_spawnattr_t*
            get() {
                return &attr;
            }
        };

        int
        create_socketpair(int sv[2]) {
#ifdef SOCK_CLOEXEC
            if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv) == 0) {
                return 0;
            }
            if (errno != EINVAL) {
                return -1;
            }
#endif
            if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
                return -1;
            }
            if ((set_cloexec(sv[0]) != 0) || (set_cloexec(sv[1]) != 0)) {
                int e = errno;
                ::close(sv[0]);
                ::close(sv[1]);
                errno = e;
                return -1;
            }
            return 0;
        }

        ssize_t
        send_no_sigpipe(int fd, const char* data, size_t size) {
#ifdef MSG_NOSIGNAL
            return send(fd, data, size, MSG_NOSIGNAL);
#else
#ifdef SO_NOSIGPIPE
            return send(fd, data, size, 0);
#else
            sigset_t block;
            sigset_t old;
            sigset_t pending;
            sigemptyset(&block);
            sigaddset(&block, SIGPIPE);
            bool blocked = false;
            bool was_pending = false;
            if (pthread_sigmask(SIG_BLOCK, &block, &old) == 0) {
                blocked = true;
                if (sigpending(&pending) == 0) {
                    was_pending = sigismember(&pending, SIGPIPE) == 1;
                }
            }
            ssize_t n = send(fd, data, size, 0);
            if ((n == -1) && (errno == EPIPE) && !was_pending) {
                const struct timespec timeout = {0, 0};
                sigtimedwait(&block, NULL, &timeout);
            }
            if (blocked) {
                pthread_sigmask(SIG_SETMASK, &old, NULL);
            }
            return n;
#endif
#endif
        }
        class PosixProcessSession : public BlackBoxProcessSession {
        protected:
            pid_t child;
            int pipe_send;
            FILE* file_receive;

            static std::string
            last_error(const std::string& prefix) {
                return prefix + " (errno " + std::to_string(errno) + ")";
            }

            static void
            sleep_grace_period() {
                struct timespec remaining = {0, 10000000};
                while ((nanosleep(&remaining, &remaining) == -1) && (errno == EINTR)) {
                }
            }

            static bool
            child_exited(pid_t pid) {
                siginfo_t info;
                do {
                    info.si_pid = 0;
                    if (waitid(P_PID, pid, &info, WEXITED | WNOHANG | WNOWAIT) == 0) {
                        return info.si_pid != 0;
                    }
                } while (errno == EINTR);
                return false;
            }

            static void
            signal_group(pid_t pid, int signal) {
                if ((kill(-pid, signal) == -1) && (errno == ESRCH)) {
                    return;
                }
            }

            static void
            wait_group(pid_t pid, int attempts) {
                for (int i = 0; i < attempts; i++) {
                    if ((kill(-pid, 0) == -1) && (errno == ESRCH)) {
                        return;
                    }
                    if (child_exited(pid)) {
                        return;
                    }
                    sleep_grace_period();
                }
            }

            static void
            terminate_child(pid_t pid) {
                if (pid <= 0) {
                    return;
                }
                int status = 0;
                // Keep the child unreaped until the group has received both signals.
                signal_group(pid, SIGTERM);
                wait_group(pid, 100);
                signal_group(pid, SIGKILL);
                do {
                    if (waitpid(pid, &status, 0) != -1) {
                        return;
                    }
                } while (errno == EINTR);
            }

            static void
            check_sigchld() {
                struct sigaction action;
                if (sigaction(SIGCHLD, nullptr, &action) != 0) {
                    throw Error("BlackBoxExec", last_error("SIGCHLD query failed"));
                }
                if ((action.sa_handler != SIG_DFL)
#ifdef SA_NOCLDWAIT
                    || (action.sa_flags & SA_NOCLDWAIT)
#endif
                ) {
                    throw Error("BlackBoxExec", "Cannot start a blackbox process unless SIGCHLD uses "
                                                "SIG_DFL without SA_NOCLDWAIT");
                }
            }

            void open_posix(const std::string& program, const std::vector<std::string>& args);
            void close_posix();

        public:
            PosixProcessSession(const std::string& program, const std::vector<std::string>& args)
                : child(-1), pipe_send(-1), file_receive(nullptr) {
                open_posix(program, args);
            }

            ~PosixProcessSession() override {
                close();
            }

            std::string
            exchange(const std::string& out_buf) override {
                const char* p = out_buf.c_str();
                size_t remaining = out_buf.size();
                while (remaining > 0) {
                    ssize_t n = send_no_sigpipe(pipe_send, p, remaining);
                    if (n < 0) {
                        if (errno == EINTR) {
                            continue;
                        }
                        throw Error("BlackBoxExec",
                                    "Writing blackbox process input failed with errno " + std::to_string(errno));
                    }
                    if (n == 0) {
                        throw Error("BlackBoxExec", "Writing blackbox process input wrote zero bytes");
                    }
                    p += n;
                    remaining -= static_cast<size_t>(n);
                }

                std::string in_buffer;
                while (true) {
                    errno = 0;
                    int ch = fgetc(file_receive);
                    if (ch == EOF) {
                        if (feof(file_receive)) {
                            throw Error("BlackBoxExec", "Blackbox process provided an incomplete response");
                        }
                        int err = errno;
                        if (err == EINTR) {
                            clearerr(file_receive);
                            continue;
                        }
                        throw Error("BlackBoxExec", std::string("Reading blackbox process output from pipe "
                                                                "failed with errno ") +
                                                        std::to_string(err));
                    }
                    in_buffer += static_cast<char>(ch);
                    if (in_buffer.size() > max_exec_response_size) {
                        throw Error("BlackBoxExec", "Blackbox process response exceeds the size limit");
                    }
                    if (ch == '\n') {
                        break;
                    }
                }
                return in_buffer;
            }

            void
            close() {
                close_posix();
            }
        };

        void
        PosixProcessSession::open_posix(const std::string& program, const std::vector<std::string>& args) {
            const int read = 0;
            const int write = 1;

            std::vector<char*> argv;
            argv.reserve(args.size() + 2);
            argv.push_back(const_cast<char*>(program.c_str()));
            for (const std::string& a : args) {
                argv.push_back(const_cast<char*>(a.c_str()));
            }
            argv.push_back(nullptr);

            check_sigchld();

            FileDescriptor child_in[2];
            FileDescriptor child_out[2];
            int fds[2];
            if (create_socketpair(fds) != 0) {
                throw Error("BlackBoxExec", last_error("stdin socket creation failed"));
            }
            child_in[read].reset(fds[read]);
            child_in[write].reset(fds[write]);
            if (create_socketpair(fds) != 0) {
                throw Error("BlackBoxExec", last_error("stdout socket creation failed"));
            }
            child_out[read].reset(fds[read]);
            child_out[write].reset(fds[write]);
            FileDescriptor* session_fds[] = {&child_in[read], &child_in[write], &child_out[read], &child_out[write]};
            for (FileDescriptor* fd : session_fds) {
                if (move_away_from_standard_fd(*fd) == -1) {
                    throw Error("BlackBoxExec", last_error("moving session descriptors away from stdio "
                                                           "failed"));
                }
            }

            SpawnFileActions actions;
            int err = actions.init();
            if (err != 0) {
                errno = err;
                throw Error("BlackBoxExec", last_error("spawn file action init failed"));
            }

            SpawnAttributes attr;
            err = attr.init();
            if (err != 0) {
                errno = err;
                throw Error("BlackBoxExec", last_error("spawn attribute init failed"));
            }

            err = posix_spawnattr_setpgroup(attr.get(), 0);
            if (err == 0) {
                short flags = POSIX_SPAWN_SETPGROUP;
#if defined(GECODE_HAS_POSIX_SPAWN_CLOEXEC_DEFAULT) && defined(GECODE_HAS_POSIX_SPAWN_ADDINHERIT_NP)
                flags |= POSIX_SPAWN_CLOEXEC_DEFAULT;
#endif
                err = posix_spawnattr_setflags(attr.get(), flags);
            }
            if (err == 0) {
                err = posix_spawn_file_actions_adddup2(actions.get(), child_in[read].get(), STDIN_FILENO);
            }
            if (err == 0) {
                err = posix_spawn_file_actions_adddup2(actions.get(), child_out[write].get(), STDOUT_FILENO);
            }
#if defined(GECODE_HAS_POSIX_SPAWN_CLOEXEC_DEFAULT) && defined(GECODE_HAS_POSIX_SPAWN_ADDINHERIT_NP)
            if (err == 0) {
                err = posix_spawn_file_actions_addinherit_np(actions.get(), STDERR_FILENO);
            }
#elifdef GECODE_HAS_POSIX_SPAWN_ADDCLOSEFROM_NP
            if (err == 0) {
                err = posix_spawn_file_actions_addclosefrom_np(actions.get(), STDERR_FILENO + 1);
            }
#endif
            if (err == 0) {
                err = posix_spawnp(&child, program.c_str(), actions.get(), attr.get(), argv.data(), environ);
            }
            if (err != 0) {
                child = -1;
                errno = err;
                throw Error("BlackBoxExec", last_error("starting blackbox process failed"));
            }

            child_in[read].reset();
            child_out[write].reset();

#ifdef SO_NOSIGPIPE
            int nosigpipe = 1;
            if (setsockopt(child_in[write].get(), SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe)) != 0) {
                int e = errno;
                terminate_child(child);
                child = -1;
                errno = e;
                throw Error("BlackBoxExec", last_error("SO_NOSIGPIPE setup failed"));
            }
#endif
            FILE* receive = fdopen(child_out[read].get(), "r");
            if (receive == nullptr) {
                int e = errno;
                terminate_child(child);
                child = -1;
                errno = e;
                throw Error("BlackBoxExec", last_error("fdopen failed"));
            }
            file_receive = receive;
            child_out[read].release();
            pipe_send = child_in[write].release();
        }

        void
        PosixProcessSession::close_posix() {
            if (pipe_send != -1) {
                ::close(pipe_send);
                pipe_send = -1;
            }
            if (file_receive != nullptr) {
                fclose(file_receive);
                file_receive = nullptr;
            }
            if (child > 0) {
                terminate_child(child);
                child = -1;
            }
        }

    } // namespace

    BlackBoxProcessSession*
    create_blackbox_process(const std::string& program, const std::vector<std::string>& args) {
        return new PosixProcessSession(program, args);
    }

} // namespace Gecode::FlatZinc
#endif

// STATISTICS: flatzinc-other
