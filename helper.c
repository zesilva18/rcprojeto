bzero(password, sizeof(password));
        nread = read(client_fd, buffer_rececao, sizeof(buffer_rececao));
        strcpy(password, buffer_rececao);
        bzero(buffer_rececao, sizeof(buffer_rececao));