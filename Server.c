#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/mman.h>

#define PORT 8080
#define MSG_LENGTH 256
#define MAX_CLIENTS 10

typedef struct {
    char messages[100][MSG_LENGTH]; // Stocke jusqu'à 100 messages
    int message_count;
    pthread_mutex_t mutex;
} SharedMemory;

SharedMemory *shm;

void *client_handler(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    char buffer[MSG_LENGTH];

    // Réception des messages du client
    while (1) {
        bzero(buffer, MSG_LENGTH);
        int read_size = recv(client_sock, buffer, MSG_LENGTH, 0);
        if (read_size > 0) {
            pthread_mutex_lock(&shm->mutex);
            strcpy(shm->messages[shm->message_count], buffer);
            shm->message_count++;
            pthread_mutex_unlock(&shm->mutex);

            printf("Message reçu: %s\n", buffer);
        } else if (read_size == 0) {
            break; // Client déconnecté
        }
    }

    close(client_sock);
    free(socket_desc);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // Configuration de la mémoire partagée
    shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shm->message_count = 0;
    pthread_mutex_init(&shm->mutex, NULL);

    // Création du socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors du bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Erreur lors du listen");
        exit(EXIT_FAILURE);
    }

    // Acceptation des connexions des clients
    while ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        pthread_t client_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        pthread_create(&client_thread, NULL, client_handler, (void *)new_sock);
        pthread_detach(client_thread);
    }

    // Nettoyage
    pthread_mutex_destroy(&shm->mutex);
    munmap(shm, sizeof(SharedMemory));

    return 0;
}
