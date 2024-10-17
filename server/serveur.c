#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_MESSAGES 100

// Structure pour un message dans la mémoire partagée
typedef struct {
    char messages[MAX_MESSAGES][BUFFER_SIZE]; // Stockage des messages
    int message_count; // Nombre de messages
    pthread_mutex_t mutex; // Mutex pour protéger l'accès
} shared_memory_t;

// Structure pour stocker les informations des clients
typedef struct {
    int socket;
    struct sockaddr_in address;
    pthread_t thread_id;
} client_t;

// Tableau des clients connectés
client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Pointeur vers la mémoire partagée
shared_memory_t *shared_mem = NULL;

// Fonction pour diffuser un message à tous les clients
void broadcast_message(const char *message) {
    pthread_mutex_lock(&shared_mem->mutex);

    // Ajouter le message dans la mémoire partagée
    if (shared_mem->message_count < MAX_MESSAGES) {
        strcpy(shared_mem->messages[shared_mem->message_count], message);
        shared_mem->message_count++;
    } else {
        // Si le buffer est plein, remplacer le plus ancien message
        for (int i = 1; i < MAX_MESSAGES; i++) {
            strcpy(shared_mem->messages[i - 1], shared_mem->messages[i]);
        }
        strcpy(shared_mem->messages[MAX_MESSAGES - 1], message);
    }

    pthread_mutex_unlock(&shared_mem->mutex);

    // Envoyer le message à tous les clients
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            send(clients[i]->socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Fonction gérée par chaque thread client
void *handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    client_t *cli = (client_t *)arg;

    // Envoyer l'historique des messages au client nouvellement connecté
    pthread_mutex_lock(&shared_mem->mutex);
    for (int i = 0; i < shared_mem->message_count; i++) {
        send(cli->socket, shared_mem->messages[i], strlen(shared_mem->messages[i]), 0);
    }
    pthread_mutex_unlock(&shared_mem->mutex);

    // Boucle pour gérer les messages entrants
    while (1) {
        int receive = recv(cli->socket, buffer, BUFFER_SIZE, 0);
        if (receive > 0) {
            buffer[receive] = '\0';
            printf("Client %d: %s\n", cli->socket, buffer);
            broadcast_message(buffer);  // Diffuser à tous les clients
        } else if (receive == 0 || strcmp(buffer, "exit") == 0) {
            printf("Client %d disconnected\n", cli->socket);
            close(cli->socket);
            break;
        }
    }

    // Libérer la mémoire et le thread
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == cli) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    free(cli);
    pthread_exit(NULL);
}

// Fonction pour démarrer le serveur
int start_server(int port) {
    int server_fd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Créer la socket du serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Initialiser l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Lier la socket à l'adresse
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors du bind");
        exit(EXIT_FAILURE);
    }

    // Ecouter les connexions
    if (listen(server_fd, 10) < 0) {
        perror("Erreur lors de l'écoute");
        exit(EXIT_FAILURE);
    }

    printf("Serveur démarré sur le port %d\n", port);

    // Initialiser la mémoire partagée
    key_t key = ftok("shmfile", 65); // Générer une clé unique
    int shmid = shmget(key, sizeof(shared_memory_t), 0666 | IPC_CREAT); // Créer la mémoire partagée
    shared_mem = (shared_memory_t *)shmat(shmid, NULL, 0); // Attacher la mémoire partagée

    // Initialiser les données dans la mémoire partagée
    shared_mem->message_count = 0;
    pthread_mutex_init(&shared_mem->mutex, NULL);

    // Boucle pour accepter et gérer les clients
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_socket < 0) {
            perror("Erreur lors de l'acceptation");
            continue;
        }

        // Créer une structure client et un thread pour chaque client
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->socket = new_socket;
        cli->address = client_addr;

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == NULL) {
                clients[i] = cli;
                pthread_create(&cli->thread_id, NULL, handle_client, (void *)cli);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        printf("Nouveau client connecté : %d\n", new_socket);
    }

    // Détacher et supprimer la mémoire partagée
    shmdt(shared_mem);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

// Fonction principale
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    start_server(port);

    return 0;
}
