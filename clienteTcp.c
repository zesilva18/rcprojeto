#include "header.h"


void menu(int fd){

    int option = 0;

    char aux_user[1024];
    char aux_pass[1024];
    char buffer[1024];
    long nread;

    struct sockaddr_in addr_multicast;
    int addrlen, sock, cnt;
    struct ip_mreq mreq;
    char message_notification[1024];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }


    int multicastTTL = 255;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &multicastTTL, sizeof(multicastTTL)) < 0) {
        perror("socket opt");
        exit(1);
    }


    bzero((char *) &addr_multicast, sizeof(addr_multicast));
    addr_multicast.sin_family = AF_INET;
    addr_multicast.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_multicast.sin_port = htons(6000);
    addrlen = sizeof(addr_multicast);

    while (1) {

        printf("===== LOGIN =====\n");
        printf("Insira o username:");
        scanf("%s", aux_user);
        printf("Insira a password:");
        scanf("%s", aux_pass);
        printf("=================\n");

        printf("USER: %s\nPass: %s\n", aux_user, aux_pass);


        write(fd, aux_user, sizeof(aux_user));
        write(fd, aux_pass, sizeof(aux_pass));

        nread = read(fd, buffer, sizeof(buffer));
        printf("buffer = %s\n", buffer);

        if (strncmp(buffer, "LOGIN BEM SUCEDIDO", strlen("LOGIN BEM SUCEDIDO")) == 0) {
            // Login mal sucedido
            break;
        }

    }

}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}

int main(int argc, char *argv[]) {

    fflush(stdout);
    int fd;
    char endServer[100];
    struct sockaddr_in addr;
    struct hostent *hostPtr;


    if(argc != 3) {
        printf("Deve comear com ip e porto do servidor\n");
        exit(1);
    }

    strcpy(endServer, argv[1]);

    //argv[1] = must be 127.0.0.1 and argv[2] = 9876

    if((hostPtr = gethostbyname(endServer)) == 0) {
        printf("Nao consegui obter endereço\n");
        exit(2);
    }

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *) (hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[2]));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        erro("socket");
    }
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        erro("Erro no bind");
    }
    printf("Conected\n");


    menu(fd);

    close(fd);
    exit(0);
}


// gcc -o client clienteTcp.c; ./client 127.0.0.1 9000