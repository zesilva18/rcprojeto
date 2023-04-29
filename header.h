//make a header 
#ifndef HEADER_H
#define HEADER_H

//include the libraries

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include "stdbool.h"
#include <signal.h>


//define the constants

#define TAM 1024
#define PORT 9876   // Port
#define BUFLEN 1024 // Buffer size
#define NPARAMETERS 10
#define MAXUSERS 100
#define MAX_LINE_LENGTH 256

//create struct with name, password and role

typedef struct user{
    char name[TAM];
    char password[TAM];
    char role[TAM];
} user;

typedef struct topico{
    char id[TAM];
    char titulo[TAM];
    //create a array with users that subscribe the topic
    char users[MAXUSERS][TAM];
    char noticias[MAXUSERS][TAM];
    int endereco_Multicast;
} topico;

//create lista ligada de users 

typedef struct {

    user users[MAXUSERS];
    topico topicos[MAXUSERS];

    bool isOnline;

} shared_memory;

extern int shmid;
extern sem_t *semshmid;


//close header

#endif