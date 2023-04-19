#include "header.h"

int shmid;
shared_memory *shm;
sem_t *semshmid;

void add_user(const char *name, const char *password, const char *role) {
    user *new_user = malloc(sizeof(user));
    if (new_user == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }
    strncpy(new_user->name, name, TAM);
    strncpy(new_user->password, password, TAM);
    strncpy(new_user->role, role, TAM);
    new_user->next = NULL;
    
    if (shm->head == NULL) {
        shm->head = new_user;
    } else {
        user *current = shm->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_user;
    }
}

// funtion to remove user from shared memory

void remove_user(const char *name) {
    user *current = shm->head;
    user *previous = NULL;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (previous == NULL) {
                shm->head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

// function to find a user name in shared memory if exist return boolean

int find_user(const char *name) {
    user *current = shm->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}


void getconfig(const char *configfile)
{
    //get on config.txt and save on struct shm the values 

    //open file

    FILE* fp = fopen(configfile, "r");
    
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    
    char line[MAX_LINE_LENGTH];
    size_t len = 0;
    ssize_t read;

    
    
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        char *name, *password, *role;
        // Use strtok to split the line into name, password, and role
        name = strtok(line, ";");
        password = strtok(NULL, ";");
        role = strtok(NULL, "\n");
        // Only print lines that contain all three elements

        //printf("aqui");

        add_user(name, password, role);
        
    }
    

}

void printSharedMemory(){
    //print shared memory
    printf("USERS DISPONIVEIS:\n");
    user *aux = shm->head;
    while(aux != NULL){
        printf("name: %s password: %s role: %s\n", aux->name, aux->password, aux->role);
        aux = aux->next;
    }
}

void erro(char *s)
{
    perror(s);
    exit(1);
}


int main(void)
{
    int sockfd;

    struct sockaddr_in si_minha, si_outra, dest_addr;

    int s, recv_len;
    socklen_t slen = sizeof(si_outra);
    char buf[BUFLEN];
    

    //init shared memory
    if ((shmid = shmget(IPC_PRIVATE, sizeof(shared_memory) + sizeof(user)*20, IPC_CREAT | 0777)) < 0) { 
        perror("shmget");
        exit(1);
    }

    //attach shared memory
    if ((shm = (shared_memory *)shmat(shmid, NULL, 0)) == (shared_memory *) -1) {
        perror("shmat");
        exit(1);
    }

    // Inicializa o semáforo
    if ((semshmid = sem_open("SEM_SHM", O_CREAT, 0777, 1)) == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    getconfig("config.txt");

    //printSharedMemory();

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Cria um socket para recepção de pacotes UDP
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        erro("Erro na criação do socket");
    }
    //printf("\nMenu:\nADD_USER {username} {password}\nDEL {username}\nLIST\nQUIT\nQUIT_SERVER\n\n");
    printf("Bem vindo ao servidor\n");

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    inet_aton("127.0.0.1", &dest_addr.sin_addr);

    // Preenchimento da socket address structure
    si_minha.sin_family = AF_INET;
    si_minha.sin_port = htons(PORT);
    si_minha.sin_addr.s_addr = htonl(INADDR_ANY);


    // Associa o socket à informação de endereço
    if (bind(s, (struct sockaddr *)&si_minha, sizeof(si_minha)) == -1)
    {
        erro("Erro no bind");
    }

    while (1)
    {

        //printf("\nMenu\nADD_USER {username} {password} {role}\nDEL {username}\nLIST\nQUIT\nQUIT_SERVER\n\n",inet_ntoa(si_outra.sin_addr), ntohs(si_outra.sin_port));
        
        char* msg = "ola\n";
        
        sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&si_outra, slen);
 

        // Espera recepção de mensagem (a chamada é bloqueante)
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_outra, (socklen_t *)&slen)) == -1)
        {
            erro("Erro no recvfrom");
        }
        // Para ignorar o restante conteúdo (anterior do buffer)

        buf[recv_len] = '\0';
        //printf("Recebido = %s", buf);
        char bufInstructions[NPARAMETERS][TAM];
        int aux = 0;

        char *token = strtok(buf, " ");

        while (token != NULL)
        {
            strcpy(bufInstructions[aux], token);
            aux++;
            //print token
            token = strtok(NULL, " ");

        }
        if (strcmp(bufInstructions[0], "ADD_USER") == 0)
        {
            //printf("aux = %d", aux);
            if (aux == 4)
            {
                printf("Adicionando um novo user com o nome %s, password %s role %s\n", bufInstructions[1], bufInstructions[2], bufInstructions[3]);
                bufInstructions[3][strlen(bufInstructions[3]) - 1] = '\0';
                add_user(bufInstructions[1], bufInstructions[2], bufInstructions[3]);
                sendto(s, "User adicionado com sucesso\n", strlen("User adicionado com sucesso\n"), 0, (struct sockaddr *)&si_outra, slen);
            }
            else
            {
                printf("ERRO!!!\nUtilize o comando (ADD_USER) com a seguinte formatação ---> ADD_USER {username} {password}\n");
            }
        }
        else if (strcmp(bufInstructions[0], "DEL") == 0)
        {
            //printf("aux = %d", aux);
            if (aux == 2)
            {
                int user = 0;
                //printf("Apagando user %s\n", bufInstructions[1]);
                bufInstructions[1][strlen(bufInstructions[1]) - 1] = '\0';
                user = find_user(bufInstructions[1]);
                if (user == 0)
                {
                    sendto(s, "User nao encontrado\n", strlen("User nao encontrado\n"), 0, (struct sockaddr *)&si_outra, slen);
                }
                else
                {
                    sendto(s, "User apagado\n", strlen("User apagado\n"), 0, (struct sockaddr *)&si_outra, slen);
                    remove_user(bufInstructions[1]);
                }
                
            }
            else
            {
                printf("ERRO!!!\nUtilize o comando (DEL) com a seguinte formatação ---> DEL {username}\n");
            }
        }
        else if (strcmp(bufInstructions[0], "LIST\n") == 0)
        {
            //printf("aux = %d\n", aux);
            if (aux == 1)
            {
                
                sendto(s, "USERS DISPONIVEIS: " , strlen("USERS DISPONIVEIS: "), 0, (struct sockaddr *)&si_outra, slen);

                user *aux = shm->head;
                while(aux != NULL){
                    
                    
                    sendto(s, " NOME: " , strlen(" NOME: "), 0, (struct sockaddr *)&si_outra, slen);
                    sendto(s, aux->name , strlen(aux->name), 0, (struct sockaddr *)&si_outra, slen);
                    sendto(s, " PASSWORD: " , strlen(" PASSWORD: "), 0, (struct sockaddr *)&si_outra, slen);
                    sendto(s, aux ->password , strlen(aux->password), 0, (struct sockaddr *)&si_outra, slen);
                    sendto(s, " ROLE: " , strlen(" ROLE: "), 0, (struct sockaddr *)&si_outra, slen);
                    sendto(s, aux->role , strlen(aux->role), 0, (struct sockaddr *)&si_outra, slen);
                    sendto(s, "\n", strlen("\n"), 0, (struct sockaddr *)&si_outra, slen);
                    
                    aux = aux->next;

                }
            }
            else
            {
                printf("ERRO!!!\nUtilize o comando (LIST) com a seguinte formatação ---> LIST\n");
            }
        }
        else if (strcmp(bufInstructions[0], "QUIT\n") == 0)
        {
            //printf("aux = %d", aux);
            if (aux == 1)
            {
                printf("Desligando do cliente\n");
            }
            else
            {
                printf("ERRO!!!\nUtilize o comando (QUIT) com a seguinte formatação ---> QUIT\n");
            }
        }
        else if (strcmp(bufInstructions[0], "QUIT_SERVER\n") == 0)
        {
            //printf("aux = %d", aux);
            if (aux == 1)
            {
                printf("A sair do servidor\n");

                return 0;
            }
            else
            {
                printf("ERRO!!!\nUtilize o comando (QUIT_SERVER) com a seguinte formatação ---> QUIT_SERVER\n");
            }
        }
        //printf("\nMenu\nADD_USER {username} {password}\nDEL {username}\nLIST\nQUIT\nQUIT_SERVER\n\n");
    }
    // Fecha socket e termina programa
    close(s);
    close(sockfd);

}