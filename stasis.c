#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "data.h"

int send_msg(int sock, int id, int op) {
        net_msg msg;
        char *msg_string;

        msg = build_msg(op, id);
        msg_string = stringify_msg(msg);

        if(send(sock, msg_string, strlen(msg_string), 0) < 0) {
                free(msg_string);
                return -1;
        }

        free(msg_string);
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

        if(send_msg(sock, atoi(argv[2]), 47) < 0) {
                perror("guru meditation");
                close(sock);
                exit(1);
        }

        for(;;) {
                sleep(60);

                if(send_msg(sock, atoi(argv[2]), 73) < 0) {
                        perror("guru meditation");
                        close(sock);
                        exit(1);
                }
        }
}