#include "header.h"

int shmid;
sem_t *semshmid;
shared_memory *shm;

int PORT_CONFIG; // colocar 9876 --concexao udp
int PORT_NOTICIAS; //colocar 9000

void terminate(){

    if (shmdt(shm) < 0) {
        perror("shmdt");
        exit(1);
    }

    // remove shared memory

    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl");
        exit(1);
    }

    // remove semaphore

    if (sem_close(semshmid) < 0) {
        perror("sem_close");
        exit(1);
    }

    if (sem_unlink("SEM_SHM") < 0) {
        perror("sem_unlink");
        exit(1);
    }

}


void erro(char *s) {
    perror(s);
    exit(1);
}

bool loginCheck(const char *name, const char *password){

    //function to check if the user is in the shared memory, check if name and password are correct and return true or false

    sem_wait(semshmid);

    int index = -1;
    for (int i = 0; i < MAXUSERS; i++) {
        if (strcmp(shm->users[i].name, name) == 0 && strcmp(shm->users[i].password, password) == 0 && strcmp(shm->users[i].role, "admin") == 0) {
            index = i;
            //printf("LOGIN FEITO\n");
            sem_post(semshmid);
            return true;
        }
    }

    if (index == -1) {
        // User not found
        printf("Error: User not found\n");
        sem_post(semshmid);
        return false;
    }

    sem_post(semshmid);
    
    
}

void delete_user(const char *name) {

    sem_wait(semshmid);

    // Find the user with the given name
    int index = -1;
    for (int i = 0; i < MAXUSERS; i++) {
        if (strcmp(shm->users[i].name, name) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        // User not found
        fprintf(stderr, "Error: User not found\n");
        return;
    }

    // Remove the user from the users array
    shm->users[index].name[0] = '\0';

    sem_post(semshmid);
}

int find_user(const char *name) {

    sem_wait(semshmid);

    // Find the user with the given name
    int index = -1;
    for (int i = 0; i < MAXUSERS; i++) {
        if (strcmp(shm->users[i].name, name) == 0) {
            index = i;
            printf("User found\n");
            sem_post(semshmid);
            return 1;
        }
    }

    if (index == -1) {
        // User not found
        printf("Error: User not found\n");
        sem_post(semshmid);
        return 0;
    }

    sem_post(semshmid);

    
}

void add_user(const char *name, const char *password, const char *role) {

    sem_wait(semshmid);

    user new_user = {0};
    strncpy(new_user.name, name, TAM - 1);
    strncpy(new_user.password, password, TAM - 1);
    strncpy(new_user.role, role, TAM - 1);

    // Find the next available index in the users array
    int index = -1;
    for (int i = 0; i < MAXUSERS; i++) {
        if (shm->users[i].name[0] == '\0') {
            index = i;
            break;
        }
    }

    if (index == -1) {
        // Users array is full
        fprintf(stderr, "Error: Users array is full\n");
        return;
    }

    // Insert the new user into the users array
    shm->users[index] = new_user;

    sem_post(semshmid);
}

void getconfig(const char *configfile) {
    // get on config.txt and save on struct shm the values

    // open file

    FILE *fp = fopen(configfile, "r");

    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        char *name, *password, *role;
        // Use strtok to split the line into name, password, and role
        name = strtok(line, ";");
        password = strtok(NULL, ";");
        role = strtok(NULL, "\n");
        // Only print lines that contain all three elements

        // printf("aqui");

        add_user(name, password, role);
    }
}

void print_shared_memory() {

    sem_wait(semshmid);

    printf("Shared Memory:\n");

    // Print each user in the users array
    for (int i = 0; i < MAXUSERS; i++) {
        if (shm->users[i].name[0] != '\0') {
            printf("User %d: Name='%s', Password='%s', Role='%s'\n", i+1, shm->users[i].name, shm->users[i].password, shm->users[i].role);
        }
    }

    sem_post(semshmid);
}

//TCP FUNCTIONS 

int loginCheckUser(const char *name, const char *password) {

    sem_wait(semshmid);

    int index = -1;
    for (int i = 0; i < MAXUSERS; i++) {
        if (strcmp(shm->users[i].name, name) == 0 && strcmp(shm->users[i].password, password) == 0 && strcmp(shm->users[i].role, "admin") != 0) {
            index = i;
            printf("LOGIN FEITO\n");
            //if user is leitor return 1 else return 2
            if (strcmp(shm->users[i].role, "leitor") == 0) {
                sem_post(semshmid);
                return 1;
            } else {
                sem_post(semshmid);
                return 2;
            }
        }
    }

    if (index == -1) {
        // User not found
        printf("Error: User not found\n");
        sem_post(semshmid);
        return 0;
    }

    sem_post(semshmid);
}

int udpConextion() {

    int sockfd;

    struct sockaddr_in si_minha, si_outra, dest_addr;

    int s, recv_len;
    socklen_t slen = sizeof(si_outra);
    char buf[BUFLEN];


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Cria um socket para recepção de pacotes UDP
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        erro("Erro na criação do socket");
    }
    printf("Bem vindo ao servidor\n");

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT_CONFIG);
    inet_aton("127.0.0.1", &dest_addr.sin_addr);

    // Preenchimento da socket address structure
    si_minha.sin_family = AF_INET;
    si_minha.sin_port = htons(PORT_CONFIG);
    si_minha.sin_addr.s_addr = htonl(INADDR_ANY);

    // Associa o socket à informação de endereço
    if (bind(s, (struct sockaddr *)&si_minha, sizeof(si_minha)) == -1) {
        erro("Erro no bind");
    }

    bool login = false;

    

        while (1) {
        // printf("\nMenu\nADD_USER {username} {password} {role}\nDEL {username}\nLIST\nQUIT\nQUIT_SERVER\n\n",inet_ntoa(si_outra.sin_addr), ntohs(si_outra.sin_port));

        // Espera recepção de mensagem (a chamada é bloqueante)
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_outra, (socklen_t *)&slen)) == -1) {
            erro("Erro no recvfrom");
        }
        // Para ignorar o restante conteúdo (anterior do buffer)

        buf[recv_len] = '\0';
        // printf("Recebido = %s", buf);
        char bufInstructions[NPARAMETERS][TAM];
        int aux = 0;

        char *token = strtok(buf, " ");

        if (!login) {
            while (token != NULL) {
                strcpy(bufInstructions[aux], token);
                aux++;
                // print token
                token = strtok(NULL, " ");
            }
            sendto(s, "Escreva username password: \n", strlen("Escreva username password: \n"), 0, (struct sockaddr *)&si_outra, slen);
            //print aux
            //printf("aux = %d\n", aux);
            if(aux == 2){

                bufInstructions[1][strlen(bufInstructions[1]) - 1] = '\0';
                printf("%s\n", bufInstructions[0]);
                printf("%s\n", bufInstructions[1]);

                if(loginCheck(bufInstructions[0], bufInstructions[1])){
                    sendto(s, "Login efetuado com sucesso\n", strlen("Login efetuado com sucesso\n"), 0, (struct sockaddr *)&si_outra, slen);
                    login = true;
                } else {
                    sendto(s, "Login errado\n", strlen("Login errado\n"), 0, (struct sockaddr *)&si_outra, slen);
                }
            }

            //sendto(s, "Escreva username password: \n", strlen("Escreva username password: \n"), 0, (struct sockaddr *)&si_outra, slen);
        }
        
        
        if (login) {

            while (token != NULL) {
                strcpy(bufInstructions[aux], token);
                aux++;
                // print token
                token = strtok(NULL, " ");
            }

            if (strcmp(bufInstructions[0], "ADD_USER") == 0) {
                // printf("aux = %d", aux);

                bufInstructions[3][strlen(bufInstructions[3]) - 1] = '\0';

                if (aux == 4 && strcmp(bufInstructions[3], "leitor") == 0 || strcmp(bufInstructions[3], "jornalista") == 0 || strcmp(bufInstructions[3], "admin") == 0) {
                    printf("Adicionando um novo user com o nome %s, password %s role %s\n", bufInstructions[1], bufInstructions[2], bufInstructions[3]);
                    add_user(bufInstructions[1], bufInstructions[2], bufInstructions[3]);
                    sendto(s, "User adicionado com sucesso\n", strlen("User adicionado com sucesso\n"), 0, (struct sockaddr *)&si_outra, slen);
                    print_shared_memory();
                } else {
                    sendto(s, "ERRO!!!\nUtilize o comando (ADD_USER) com o seguinte formato: ADD_USER {username} {password} {role}\n", strlen("ERRO!!!\nUtilize o comando (ADD_USER) com o seguinte formato: ADD_USER {username} {password} {role}\n"), 0, (struct sockaddr *)&si_outra, slen);
                    sendto(s, "De relembrar que um user so pode ter role de leitor ou jornalista\n", strlen("De relembrar que um user so pode ter role de leitor, escritor ou admin\n"), 0, (struct sockaddr *)&si_outra, slen);
                }
            } else if (strcmp(bufInstructions[0], "DEL") == 0) {
                // printf("aux = %d", aux);
                if (aux == 2) {
                    int user = 0;
                    // printf("Apagando user %s\n", bufInstructions[1]);
                    bufInstructions[1][strlen(bufInstructions[1]) - 1] = '\0';
                    user = find_user(bufInstructions[1]);
                    if (user == 0) {
                        sendto(s, "User nao encontrado\n", strlen("User nao encontrado\n"), 0, (struct sockaddr *)&si_outra, slen);
                    } else {
                        sendto(s, "User apagado\n", strlen("User apagado\n"), 0, (struct sockaddr *)&si_outra, slen);
                        delete_user(bufInstructions[1]);
                    }

                } else {
                    sendto(s, "ERRO!!!\nUtilize o comando (DEL) com o seguinte formato: DEL {username}\n", strlen("ERRO!!!\nUtilize o comando (DEL) com o seguinte formato: DEL {username}\n"), 0, (struct sockaddr *)&si_outra, slen);
                }
            } else if (strcmp(bufInstructions[0], "LIST\n") == 0) {
                // printf("aux = %d\n", aux);
                if (aux == 1) {
                    printf("Lista de users disponiveis:\n");
                    print_shared_memory();
                    sendto(s, "USERS DISPONIVEIS: \n", strlen("USERS DISPONIVEIS: \n"), 0, (struct sockaddr *)&si_outra, slen);
                    sem_wait(semshmid);

                    // Print each user in the users array
                    for (int i = 0; i < MAXUSERS; i++) {
                    if (shm->users[i].name[0] != '\0') {

                        sendto(s, "Username: ", strlen("Username: "), 0, (struct sockaddr *)&si_outra, slen);
                        sendto(s, shm->users[i].name, strlen(shm->users[i].name), 0, (struct sockaddr *)&si_outra, slen);
                        sendto(s, " Password: ", strlen(" Password: "), 0, (struct sockaddr *)&si_outra, slen);
                        sendto(s, shm->users[i].password, strlen(shm->users[i].password), 0, (struct sockaddr *)&si_outra, slen);
                        sendto(s, " Role: ", strlen(" Role: "), 0, (struct sockaddr *)&si_outra, slen);
                        sendto(s, shm->users[i].role, strlen(shm->users[i].role), 0, (struct sockaddr *)&si_outra, slen);

                        sendto(s, "\n", strlen("\n"), 0, (struct sockaddr *)&si_outra, slen);

                    }
                }
                    sem_post(semshmid);
                } else {
                    sendto(s, "ERRO!!!\nUtilize o comando (LIST) com a seguinte formatação ---> LIST\n", strlen("ERRO!!!\nUtilize o comando (LIST) com a seguinte formatação ---> LIST\n"), 0, (struct sockaddr *)&si_outra, slen);
                }
            } else if (strcmp(bufInstructions[0], "QUIT\n") == 0) {
                // printf("aux = %d", aux);
                if (aux == 1) {
                    printf("Desligando do cliente\n");
                    sendto(s, "Desligando do cliente\n", strlen("Desligando do cliente\n"), 0, (struct sockaddr *)&si_outra, slen);
                    login = false;
                } else {
                    printf("ERRO!!!\nUtilize o comando (QUIT) com a seguinte formatação ---> QUIT\n");
                }
            } else if (strcmp(bufInstructions[0], "QUIT_SERVER\n") == 0) {
                // printf("aux = %d", aux);
                if (aux == 1) {
                    printf("A sair do servidor\n");

                    terminate();

                    return 0;
                } else {
                    printf("ERRO!!!\nUtilize o comando (QUIT_SERVER) com a seguinte formatação ---> QUIT_SERVER\n");
                }
            }
            if(login){
                 sendto(s, "\nMenu\nADD_USER {username} {password} {role}\nDEL {username}\nLIST\nQUIT\nQUIT_SERVER\n\n", strlen("\nMenu\nADD_USER {username} {password} {role}\nDEL {username}\nLIST\nQUIT\nQUIT_SERVER\n\n"), 0, (struct sockaddr *)&si_outra, slen);
            }
           

        }
    }

    // Fecha socket e termina programa
    close(s);
    close(sockfd);

}

//function to go trough array topicos, get titulos and return a char with all the titulos separete by \n

char *getTitulos(char *titulos) {

    bool found = false;
    titulos[0] = '\0';

    sem_wait(semshmid);

    for (int i = 0; i < MAXUSERS; i++) {
        if (shm->topicos[i].titulo[0] != '\0') {
            found = true;
            strcat(titulos, shm->topicos[i].titulo);
            // add a id
            strcat(titulos, " ");
            strcat(titulos, shm->topicos[i].id);
            strcat(titulos, "\n");
        }
    }

    sem_post(semshmid);

    //if found is false, clean the string titulos and put a message

    if (!found) {
        strcat(titulos, "ERRO");
    }

    return titulos;
}

//create a function to receive a id and a name and add that name to the array users in topico

bool subscribeTopico(char *id, const char *name) {

    sem_wait(semshmid);

    bool found = false;

    // find id in the array of topicos and insert on topico.users array the name

    for (int i = 0; i < MAXUSERS; i++) {
        if (strcmp(shm->topicos[i].id, id) == 0) {
            found = true;
            for (int j = 0; j < MAXUSERS; j++) {
                if (shm->topicos[i].users[j][0] == '\0') {
                    strncpy(shm->topicos[i].users[j], name, TAM - 1);
                    break;
                }
            }
            break;
        }
    }

    if (!found) {
        printf("Error: Topic not found\n");
    } else {
        printf("User %s subscribed to topic %s\n", name, id);
    }

    sem_post(semshmid);

    return found;
}

bool add_noticias(char *id, char *noticias) {
    
        bool found = false;
    
        sem_wait(semshmid);
    
        // find id in the array and put noticias in the array noticias

        for (int i = 0; i < MAXUSERS; i++) {
        if (strcmp(shm->topicos[i].id, id) == 0) {
            found = true;
            for (int j = 0; j < MAXUSERS; j++) {
                if (shm->topicos[i].noticias[j][0] == '\0') {
                    strncpy(shm->topicos[i].noticias[j], noticias, TAM - 1);
                    break;
                }
            }
            break;
        }
    }

    if (!found) {
        printf("Error: Topic not found\n");
    } else {
        printf("New noticie %s to topic %s\n", noticias, id);
    }

    
        sem_post(semshmid);
    
        return found;
}

int add_topico(char *id, char *titulo) {
    sem_wait(semshmid);

    bool found = false;

    topico new_topico = {0};
    strncpy(new_topico.id, id, TAM - 1);
    strncpy(new_topico.titulo, titulo, TAM - 1);

    // Find the next available index in the users array
    int index = -1;
    for (int i = 0; i < MAXUSERS; i++) {

        //check if the id is already in the array
        if (strcmp(shm->topicos[i].id, id) == 0) {
            found = true;
            break;
        }
        if (shm->topicos[i].id[0] == '\0') {
            index = i;
            break;
        }
    }

    if (index == -1) {
        // Users array is full
        fprintf(stderr, "Error: Users array is full\n");
        return 0;
    }

    if (found) {
        printf("Error: Topic already exists\n");
        sem_post(semshmid);
        return 1;
    } else {
        printf("Topic %s added\n", id);

        shm->topicos[index] = new_topico;

        printf("Topico adicionado com sucesso\n");

        sem_post(semshmid);

        return 2;
    }

    
}

void showTopicos () {
    sem_wait(semshmid);

    // print for each topico the id and the titulo and al users subscribed to that topico

    for (int i = 0; i < MAXUSERS; i++) {
        if (shm->topicos[i].id[0] != '\0') {

            printf("ID: %s\n", shm->topicos[i].id);
            printf("Titulo: %s\n", shm->topicos[i].titulo);

            printf("Users:\n");
            for (int j = 0; j < MAXUSERS; j++) {
                if (shm->topicos[i].users[j][0] != '\0') {
                    printf("%s\n", shm->topicos[i].users[j]);
                }
            }

            printf("Noticias:\n");
            for (int j = 0; j < MAXUSERS; j++) {
                if (shm->topicos[i].noticias[j][0] != '\0') {
                    printf("%s\n", shm->topicos[i].noticias[j]);
                }
            }

            printf("\n");
        }
    }

    sem_post(semshmid);
}   

void process_jornalista(int client_fd, const char *name, const char *password){
    char buffer_rececao[BUFLEN];
    long nread;
    bool login = false;

    printf("Jornalista %s logged in\n", name);

    while (!login) {

        bzero(buffer_rececao, sizeof(buffer_rececao));
        nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));

        printf("buffer rececao: %s\n", buffer_rececao);

        if (strncmp("LIST", buffer_rececao, strlen("LIST")) == 0){
            printf("OPCAO LISTAR\n");

            char *titulos = malloc(sizeof(char) * 1024);

            titulos = getTitulos(titulos);

            //print the titulos

            printf("%s", titulos);

            write(client_fd, titulos, strlen(titulos));

            showTopicos();

            free(titulos);
            
        }else if (strncmp("SUBS", buffer_rececao, strlen("SUBS")) == 0){
            printf("OPCAO SUBSCREVER\n");
            char idTopico[1024];

            bzero(idTopico, sizeof(idTopico));
            nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
            strcpy(idTopico, buffer_rececao);
            bzero(buffer_rececao, sizeof(buffer_rececao));

            if(subscribeTopico(idTopico, name)) {
                write(client_fd, "OK", sizeof("OK"));
                showTopicos();
            } else {
                write(client_fd, "ERRO", sizeof("ERRO"));
            }

        }else if (strncmp("CRT", buffer_rececao, strlen("CRT")) == 0){
            printf("CRIAR TOPICO\n");
            
            char idTopico[1024];
            char tituloTopico[1024];
            int valido = 0;

            bzero(idTopico, sizeof(idTopico));
            nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
            strcpy(idTopico, buffer_rececao);
            bzero(buffer_rececao, sizeof(buffer_rececao));

            printf("ID: %s\n", idTopico);

            bzero(tituloTopico, sizeof(tituloTopico));
            nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
            strcpy(tituloTopico, buffer_rececao);
            bzero(buffer_rececao, sizeof(buffer_rececao));

            printf("TITULO: %s\n", tituloTopico);

            if(add_topico(idTopico, tituloTopico) == 2) {
                write(client_fd, "OK", sizeof("OK"));
                showTopicos();
            } else if(add_topico(idTopico, tituloTopico) == 1) {
                write(client_fd, "ID", sizeof("ID"));
            } else {
                write(client_fd, "ERRO", sizeof("ERRO"));
            }

        }else if (strncmp("SND", buffer_rececao, strlen("SND")) == 0){

            char idTopico[1024];
            char noticias[1024];

            bzero(idTopico, sizeof(idTopico));
            nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
            strcpy(idTopico, buffer_rececao);
            bzero(buffer_rececao, sizeof(buffer_rececao));

            printf("ID: %s\n", idTopico);

            bzero(noticias, sizeof(noticias));
            nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
            strcpy(noticias, buffer_rececao);
            bzero(buffer_rececao, sizeof(buffer_rececao));

            printf("NOTICIA: %s\n", noticias);

            if(add_noticias(idTopico, noticias)) {
                write(client_fd, "OK", sizeof("OK"));
                showTopicos();
            } else {
                write(client_fd, "ERRO", sizeof("ERRO"));
                printf("Erro ao criar noticias\n");
            }

        }else if (strncmp("EXIT", buffer_rececao, strlen("EXIT")) == 0){
            login = true;
        }

    }
}

void process_leitor(int client_fd, const char *name, const char *password){
    char buffer_rececao[BUFLEN];
    long nread;
    bool login = false;

    printf("Leitor %s logged in\n", name);

    while(!login){

        bzero(buffer_rececao, sizeof(buffer_rececao));
        nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));

        printf("buffer rececao: %s\n", buffer_rececao);

        if (strncmp("LIST", buffer_rececao, strlen("LIST")) == 0){
            printf("OPCAO LISTAR\n");

            char *titulos = malloc(sizeof(char) * 1024);

            titulos = getTitulos(titulos);

            //print the titulos

            printf("%s", titulos);

            write(client_fd, titulos, strlen(titulos));

            showTopicos();

            free(titulos);
            
        }

        else if (strncmp("SUBS", buffer_rececao, strlen("SUBS")) == 0){
            printf("OPCAO SUBSCREVER\n");
            char idTopico[1024];

            bzero(idTopico, sizeof(idTopico));
            nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
            strcpy(idTopico, buffer_rececao);
            bzero(buffer_rececao, sizeof(buffer_rececao));

            if(subscribeTopico(idTopico, name)) {
                write(client_fd, "OK", sizeof("OK"));
                showTopicos();
            } else {
                write(client_fd, "ERRO", sizeof("ERRO"));
            }

        }else if (strncmp("EXIT", buffer_rececao, strlen("EXIT")) == 0){
            login = true;
        }

    }

}

void process_client(int client_fd)
{

    char buffer_rececao[BUFLEN];
    char buffer_envio[BUFLEN];
    char name[BUFLEN];
    char password[BUFLEN];
    long nread;

    while (1) {
        bzero(name, sizeof(name));
        nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
        strcpy(name, buffer_rececao);
        bzero(buffer_rececao, sizeof(buffer_rececao));

        bzero(password, sizeof(password));
        nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
        strcpy(password, buffer_rececao);
        bzero(buffer_rececao, sizeof(buffer_rececao));


        //print name and password

        printf("User name %s with password %s\n", name, password);
        int role = loginCheckUser(name, password);
        if(role == 1){
            printf("LOGIN FEITO!\n");
            write(client_fd, "leitor", sizeof("leitor"));
            process_leitor(client_fd, name, password);
            break;
            }else if(role == 2){
                printf("LOGIN FEITO!\n");
                write(client_fd, "jornalista", sizeof("jornalista"));
                process_jornalista(client_fd, name, password);
                break;
            } 
            else if (role == 0){
                printf("LOGIN NAO FEITO!\n");
                print_shared_memory();
                write(client_fd, "LOGIN MAL SUCEDIDO", sizeof("LOGIN MAL SUCEDIDO"));
        }
    }
}

void tcpConextion(){

    printf("A iniciar o servidor TCP\n");

    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT_NOTICIAS);

    if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erro("na funcao socket");
    if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
        erro("na funcao bind");
    if( listen(fd, 5) < 0)
        erro("na funcao listen");
    client_addr_size = sizeof(client_addr);
    while (1) {
        //clean finished child processes, avoiding zombies
        //must use WNOHANG or would block whenever a child process was working
        while(waitpid(-1,NULL,WNOHANG)>0);
        //wait for new connection
        client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
        if (client > 0) {
            if (fork() == 0) {
                process_client(client);
                close(fd);
                exit(0);
            }
            close(client);
        }

    }

}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("numero de argumentos invalido!\n");
        printf("./news_server <PORTO_NOTICIAS> <PORTO_CONFIG> <ficheiro configuração>!\n");
        exit(1);
    }

    PORT_NOTICIAS = (int) strtol(argv[1], NULL, 10);
    PORT_CONFIG = (int) strtol(argv[2], NULL, 10);

    //convert argv[3] to a string

    char *configfile = argv[3];

    if (strcmp(configfile, "config.txt") != 0) {
        printf("ficheiro de configuração invalido!\n");
        exit(1);
    }

    sem_unlink("SEM_SHM");
    semshmid = sem_open("SEM_SHM", O_CREAT, 0777, 1);

    // init shared memory
    if ((shmid = shmget(IPC_PRIVATE, sizeof(shared_memory), IPC_CREAT | 0777)) < 0) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory
    if ((shm = (shared_memory *)shmat(shmid, NULL, 0)) == (shared_memory *)-1) {
        perror("shmat");
        exit(1);
    }


    // Inicializa o semáforo
    if ((semshmid = sem_open("SEM_SHM", O_CREAT, 0777, 1)) == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    
    getconfig("config.txt");

    if (fork () == 0) {
        udpConextion();
        exit(0);
    }

    if (fork () == 0) {
        tcpConextion();
        exit(0);
    }

    for (int i = 0; i < 2; i++) {
        wait(NULL);
    }

    terminate();

    return 0;

}

// gcc -o server server.c; ./server 9000 9876 config.txt
// nc -u 127.0.0.1 