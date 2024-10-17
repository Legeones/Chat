#include <stdio.h>
#define MAX_CLIENTS 10
#define MAX_MESSAGES 100
#define MSG_LENGTH 256

typedef struct {
    char messages[MAX_MESSAGES][MSG_LENGTH]; // Liste de messages
    int message_count;                       // Nombre de messages
    pthread_mutex_t mutex;                   // Mutex pour la synchronisation
} SharedMemory;

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/mman.h>

#define PORT 8080

// Structure pour la mémoire partagée
SharedMemory *shm;

void *client_handler(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    char buffer[MSG_LENGTH];

    // Réception des messages
    while (1) {
        bzero(buffer, MSG_LENGTH);
        int read_size = recv(client_sock, buffer, MSG_LENGTH, 0);
        if (read_size > 0) {
            pthread_mutex_lock(&shm->mutex);
            strcpy(shm->messages[shm->message_count], buffer);
            shm->message_count++;
            pthread_mutex_unlock(&shm->mutex);

            // Diffusion du message à tous les clients
            for (int i = 0; i < MAX_CLIENTS; i++) {
                // Logic de diffusion (ajuster selon la liste de clients actifs)
            }
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

    // Création de la mémoire partagée
    shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shm->message_count = 0;
    pthread_mutex_init(&shm->mutex, NULL);

    // Création du socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);

    // Acceptation des connexions des clients
    while ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len))) {
        pthread_t client_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        // Création d'un thread pour gérer chaque client
        pthread_create(&client_thread, NULL, client_handler, (void *)new_sock);
        pthread_detach(client_thread);
    }

    // Nettoyage
    pthread_mutex_destroy(&shm->mutex);
    munmap(shm, sizeof(SharedMemory));

    return 0;
}



int main(void) {
    printf("Hello, World!\n");
    return 0;
}
