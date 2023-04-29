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