#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "data.h"

int authenticate(int sock) {
        net_msg auth_msg;
        char *auth_string;

        auth_msg = build_msg(47, 10);
        auth_string = stringify_msg(auth_msg);

        if(send(sock, auth_string, strlen(auth_string), 0) < 0) {
                perror("guru meditation");
                close(sock);
                free(auth_string);
                return -1;
        }

        free(auth_string);
        return 0;
}

int main(int argc, char **argv) {
        int sock;
        struct sockaddr_in server;

        /* socket creation and server addressing */
        if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                perror("guru meditation");
                exit(1);
        }

        memset(&server, 0, sizeof server);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(argv[1]);
        server.sin_port = htons(9047);

        /* establish connection to server */
        if(connect(sock, (struct sockaddr *)&server, sizeof server) < 0) {
                perror("guru meditation");
                close(sock);
                exit(1);
        }

        if(authenticate(sock) < 0) {
                perror("guru meditation");
                close(sock);
                exit(1);
        }

}