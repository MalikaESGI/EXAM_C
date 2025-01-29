#ifndef INTERCEPTOR_H
#define INTERCEPTOR_H

#include <unistd.h>

ssize_t write(int fd, const void *buf, size_t count);

#endif // INTERCEPTOR_H