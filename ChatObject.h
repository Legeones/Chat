//
// Created by berna on 17/10/2024.
//

#ifndef CHATOBJECT_H
#define CHATOBJECT_H

#include <pthread.h>
#include <stdio.h>
#define MAX_CLIENTS 10
#define MAX_MESSAGES 100
#define MSG_LENGTH 256

typedef struct {
    char messages[MAX_MESSAGES][MSG_LENGTH]; // Liste de messages
    int message_count;                       // Nombre de messages
    pthread_mutex_t mutex;                   // Mutex pour la synchronisation
} SharedMemory;

#endif //CHATOBJECT_H