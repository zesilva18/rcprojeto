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

        if(loginCheckUser(name, password)){
            printf("LOGIN FEITO!\n");
            write(client_fd, "LOGIN BEM SUCEDIDO", sizeof("LOGIN BEM SUCEDIDO"));
            break;
        } 
        else {
            printf("LOGIN NAO FEITO!\n");
            printSharedMemory();
            write(client_fd, "LOGIN MAL SUCEDIDO", sizeof("LOGIN MAL SUCEDIDO"));
        }
    }
}


bool loginCheckUser(const char *name, const char *password) {
    sem_wait(semshmid);
    user *current = shm->head;

    printf("name: %s password: %s\n", name, password);
    while (current != NULL) {

        //make a print to see value of strcmp 

        printf("name: %s password: %s\n", current->name, current->password);

        printf("value of strcmp name : %d\n", strcmp(current->name, name));
        printf("value of strcmp password : %d\n", strcmp(current->name, name));
        

        if (strcmp(current->name, name) == 0 && strcmp(current->name, name)== 0) {
            sem_post(semshmid);
            return true;
        }
        current = current->next;

        
    }
    sem_post(semshmid);
    return false;
}