// gcc -o wholock wholock.c
// sudo mv wholock /usr/bin
// sudo chown root:root /usr/bin/wholock
// sudo chmod u+s /usr/bin/wholock

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define PERFLOCK "/tmp/perflock"

bool isdigits(const char *s)
{
        for (; *s; ++s)
                if (!isdigit(*s))
                        return false;
        return true;
}

void checkIOErr(const char *syscall, const char *pid)
{
        if (errno == ENOENT)
                return;
        if (errno == EACCES) {
                fprintf(stderr, "access denied for PID %s\n", pid);
                return;
        }
        perror(syscall);
        exit(1);
}

void showPID(const char *pid)
{
        char path[PATH_MAX];
        snprintf(path, sizeof path, "/proc/%s/cmdline", pid);

        // Get process command line
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
                checkIOErr("open", pid);
                return;
        }
        char cmdline[1024];
        ssize_t cmdlineLen = read(fd, cmdline, sizeof cmdline);
        if (cmdlineLen < 0) {
                perror("read");
                exit(1);
        }
        int i;
        for (i = 0; i < cmdlineLen; ++i) {
                if (cmdline[i] == '\0')
                        cmdline[i] = ' ';
        }
        cmdline[cmdlineLen - 1] = '\0';

        // Get UID
        struct stat st;
        if (fstat(fd, &st) < 0) {
                close(fd);
                checkIOErr("stat", pid);
                return;
        }
        close(fd);

        // Get user name
        struct passwd *passwd = getpwuid(st.st_uid);
        if (!passwd) {
                perror("getpwuid");
                exit(1);
        }

        printf("%-10s %-10s %s\n", passwd->pw_name, pid, cmdline);
}

void checkPID(const char *pid)
{
        char fdspath[PATH_MAX];
        snprintf(fdspath, sizeof fdspath, "/proc/%s/fd", pid);
        DIR *fds = opendir(fdspath);
        if (!fds) {
                checkIOErr("opendir", pid);
                return;
        }

        struct dirent *d;
        while ((d = readdir(fds))) {
                if (!isdigits(d->d_name))
                        continue;

                char buf[PATH_MAX];
                int size = readlinkat(dirfd(fds), d->d_name, buf, sizeof buf);
                if (size == -1) {
                        checkIOErr("readlinkat", pid);
                        continue;
                }
                if (size >= sizeof buf) {
                        fprintf(stderr, "path of FD %s in PID %s too long\n",
                                d->d_name, pid);
                        continue;
                }
                buf[size] = '\0';
                if (strcmp(buf, PERFLOCK) == 0) {
                        showPID(pid);
                        break;
                }
        }

        closedir(fds);
}

int main(int argc, char **argv)
{
        DIR *proc = opendir("/proc");
        if (!proc) {
                perror("opendir");
                exit(1);
        }

        struct dirent *d;
        while ((d = readdir(proc))) {
                if (!isdigits(d->d_name))
                        continue;

                checkPID(d->d_name);
        }

        closedir(proc);
}
