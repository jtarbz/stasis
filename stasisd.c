#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define PORT 9047
#define MAX_CLIENTS 12
#define RECV_SIZE

struct team_box {
        unsigned int team_number;
        unsigned int fd;
        unsigned int uptime;
};

/* dunno what i'm doing w/ this yet */
struct client_message {
        unsigned int team_number;
};

/* accept an auth object from a client and test it for validity */
int authenticate_client(int client_socket) {

        return 0;
}

/* authenticate and accept an incoming client */
int accept_client(int server_socket, int epoll_fd) {
        int client_socket;
        unsigned int client_length;
        struct sockaddr_in client_addr;
        struct epoll_event client_event;

        if((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_length)) < 0) {
                fprintf(stderr, "failed to accept an incoming client\n");
                return -1;
        }
    
        /* authenticate client */
        /* accept some authentification object from the client, run it through a function to test it */
        /* to do: determine auth structure, send structure via stream socket, verify function */


        /* add client socket to epoll interest list */
        client_event.events = EPOLLIN;
        client_event.data.fd = client_socket;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event) < 0) {
                perror("guru meditation");
                close(client_socket);
                return -1;
        }

        return 0;
}

int main(int argc, char **argv) {
        int server_socket, epoll_fd, number_fds, tmp_fd;
        int *client_registry = malloc(sizeof(int) * MAX_CLIENTS);
        struct sockaddr_in server_addr;
        struct epoll_event listener_event;
        struct epoll_event ready_sockets[MAX_CLIENTS + 1];	// leave room for listener
                
        memset(client_registry, 0, sizeof(int) * MAX_CLIENTS);
        
        /* spawn socket and prepare for binding */
        if((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                perror("guru meditation");
                exit(1);
        }
        
        if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
                perror("guru meditation");
                exit(1);
        }
        
        memset(&server_addr, 0, sizeof(struct sockaddr_in));    // zero out to prevent bad voodoo
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(PORT);
        
        /* bind socket and listen */
        if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof server_addr) < 0) {
                perror("guru meditation");
                exit(1);
        }
        
        if(listen(server_socket, 64) < 0) {
                perror("guru meditation");
                close(server_socket);
                exit(1);
        }
        
        /* set up epoll */
        if((epoll_fd = epoll_create(MAX_CLIENTS + 1)) < 0) {
                perror("guru meditation");
                close(server_socket);
                exit(1);
        }
        
        /* add listening socket to epoll interest list */
        listener_event.events = EPOLLIN;
        listener_event.data.fd = server_socket;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &listener_event) < 0) {
                perror("guru meditation");
                close(server_socket);
                close(epoll_fd);
                exit(1);
        }
        
        /* main program loop - verify and handle clients by activity */
        for(;;) {
                /* block until some sockets are ready */
                if((number_fds = epoll_wait(epoll_fd, ready_sockets, MAX_CLIENTS + 1, -1)) < 0) {
                        perror("guru meditation");
                        continue;
                }
                
                /* loop through ready sockets */
                for(int i = 0; i < number_fds; ++i) {
                        /* a new client is trying to connect */
                        if((tmp_fd = ready_sockets[i].data.fd) == server_socket) {
                                if((tmp_fd = accept_client(server_socket, epoll_fd)) < 0) {
                                        perror("guru meditation");
                                        continue;       // if accepting OR authentication fails, try the next ready socket
                                }
                                
                                /* add new client to registry */
                                for(int i = 0; i < MAX_CLIENTS; ++i) {
                                        if(client_registry[i] == 0) {
                                                client_registry[i] = tmp_fd;
                                                break;
                                        }
                                
                                        if(i == MAX_CLIENTS - 1) {
                                                printf("client limit exceeded: an unregistered client has been blocked\n");
                                                close(tmp_fd);   
                                        }
                                }
                        }
                        
                        /* receiving data from a client */
                        else {
                                if(handle_client(tmp_fd) < 0) {
                                        /* remove client from registry */
                                        for(int i = 0; i < MAX_CLIENTS; ++i) {
                                                if(client_registry[i] == tmp_fd) {
                                                        close(tmp_fd);
                                                        client_registry[i] = 0;
                                                        break;
                                                }
                                        }
                                }
                        }
                }
        }
}