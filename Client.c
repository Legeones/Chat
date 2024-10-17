//
// Created by berna on 17/10/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_LENGTH 256

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char message[MSG_LENGTH];

    // Création du socket client
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Connexion au serveur
    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Émission et réception des messages
    while (1) {
        printf("Message: ");
        fgets(message, MSG_LENGTH, stdin);
        send(sock, message, strlen(message), 0);
    }

    close(sock);
    return 0;
}
