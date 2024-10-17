#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// Fonction gérée par un thread pour recevoir les messages
void *receive_messages(void *socket_fd) {
    int sockfd = *((int *)socket_fd);
    char buffer[BUFFER_SIZE];

    while (1) {
        int receive = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (receive > 0) {
            buffer[receive] = '\0';
            printf("%s\n", buffer);
        } else if (receive == 0) {
            printf("Serveur déconnecté.\n");
            close(sockfd);
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <adresse_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Variables de connexion
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in server_addr;

    // Créer la socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erreur lors de la création de la socket");
        return EXIT_FAILURE;
    }

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        return EXIT_FAILURE;
    }

    printf("Connecté au serveur.\n");

    // Créer un thread pour recevoir les messages
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &sockfd);

    // Boucle pour envoyer les messages
    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        if (strncmp(message, "exit", 4) == 0) {
            break;
        }
        send(sockfd, message, strlen(message), 0);
    }

    // Fermer la connexion
    close(sockfd);

    return 0;
}
