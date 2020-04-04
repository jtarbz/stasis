#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "data.h"

#define PORT 9047
#define MAX_CLIENTS 5096
#define MAX_BOXES 12

/* structure to hold information about individual team boxes */
typedef struct {
        unsigned int team_number;
        unsigned int fd;
        unsigned int uptime;
        time_t last_time;
} team_box;

/* read the log file to update team time values */
int update_reg(team_box *reg_key, FILE *log_file)
{
        char *line = NULL;
        size_t n = 0;
        unsigned int tmp_i, tmp_up;
        time_t tmp_last;
        
        /* seek to the beginning of the log file */
        fseek(log_file, 0, SEEK_SET);

        while (getline(&line, &n, log_file) != -1) {
                sscanf(line, "%u,%u,%ld", &tmp_i, &tmp_up, &tmp_last);
                reg_key[tmp_i - 10].uptime = tmp_up;
                reg_key[tmp_i - 10].last_time = tmp_last;
        }

        free(line);     // memory is implicitly allocated in getline()
}

/* write incoming message from client socket to buffer */
int write_msg(char *recvd_string, int client_socket)
{
        /* receive packet, write to buffer, add null terminator, blah blah blah */
	for (int recvd_msg_size = 0; recvd_msg_size < (BUFSIZE -1); ++recvd_msg_size) {
		if (recv(client_socket, recvd_string + recvd_msg_size, 1, 0) < 0)
			return -1;

                if (*(recvd_string + recvd_msg_size) == '?') {
                        *(recvd_string + recvd_msg_size) = '\0';
                        return 0;
                }
        }
}

/* accept an auth message from a client and test it for validity */
int authenticate_client(net_msg recvd_msg)
{
        /* error-check all aspects of auth message */
        /* return 0 for auth, -1 for auth failure */  
        if ((recvd_msg.op + recvd_msg.id) != (recvd_msg.check ^ 'L')) return -1;
        if (recvd_msg.id < 10 || recvd_msg.id > 21) return -1;

        return 0;      
}

/* accept an inc message from a client and test it for validity */
int increment_client(net_msg recvd_msg)
{
        /* error-check the inc message */
        /* return 0 for good, -1 for failure */
        if ((recvd_msg.op + recvd_msg.id) != (recvd_msg.check ^ 'L')) return -1;
        if (recvd_msg.id < 10 || recvd_msg.id > 21) return -1;

        return 0;
}

int handle_client(int client_socket, team_box *reg_key, FILE *log_file)
{
        net_msg recvd_msg;
        char *recvd_string = malloc(BUFSIZE);
        time_t new_time;

        if (write_msg(recvd_string, client_socket) < 0) return -1;
        recvd_msg = destring_msg(recvd_string); // recvd_string is freed implicitly

        switch (recvd_msg.op) {
        case 47:
		if (authenticate_client(recvd_msg) < 0) return -2;
		for (int i = 0; i < MAX_BOXES; ++i) if(reg_key[i].fd == client_socket) return -2;
		if (reg_key[recvd_msg.id - 10].fd != 0) {
			fprintf(stderr, "double-register of client\n");
			return -2;
		}
		reg_key[recvd_msg.id - 10].fd = client_socket;

		/* DEBUG */
		printf("team %d has been authenticated\n", recvd_msg.id - 10);

		break;
        case 73:
		new_time = time(NULL);
		if (new_time - reg_key[recvd_msg.id - 10].last_time < 58) { // clients should be pinging every 60 seconds. some error is allowed
			fprintf(stderr, "team %d has too short of an interval between incs, probably cheating\n", recvd_msg.id - 10);
			return -1;
		}
		if (increment_client(recvd_msg) < 0) return -1;
		++reg_key[recvd_msg.id - 10].uptime;
		reg_key[recvd_msg.id - 10].last_time = new_time;

		/* update log file information*/
		fprintf(log_file, "%d,%d,%ld\n", recvd_msg.id, reg_key[recvd_msg.id - 10].uptime, reg_key[recvd_msg.id - 10].last_time);
		fflush(log_file);

		/* DEBUG */
		printf("team %d has points %d\n", recvd_msg.id - 10, reg_key[recvd_msg.id - 10].uptime);

		break;
        default:
		break;
        }

        return 0;
}

/* authenticate and accept an incoming client */
int accept_client(int server_socket, int epoll_fd)
{
        int client_socket;
        unsigned int client_length;
        struct sockaddr_in client_addr;
        struct epoll_event client_event;

        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_length)) < 0) {
                fprintf(stderr, "failed to accept an incoming client\n");
                return -1;
        }

        /* add client socket to epoll interest list */
        client_event.events = EPOLLIN;
        client_event.data.fd = client_socket;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event) < 0) {
                perror("guru meditation");
                close(client_socket);
                return -1;
        }

        return 0;
}

int main(int argc, char **argv)
{
	int server_socket, epoll_fd, number_fds, tmp_fd;   
        struct sockaddr_in server_addr;
        struct epoll_event listener_event;
        struct epoll_event ready_sockets[MAX_CLIENTS + 1];      // leave room for listener
        team_box *reg_key = malloc(sizeof(team_box) * MAX_BOXES);
        FILE *log_file;
                
        /* clear and initialize registered client array */
        memset(reg_key, 0, sizeof(team_box) * MAX_BOXES);
        for (int i = 0; i < MAX_BOXES; ++i) reg_key[i].team_number = (i + 10);
        
        /* open log file */
        if ((log_file = fopen("./log_file", "r+")) == NULL) {
                perror("guru meditation");
                exit(1);
        }

        /* update client registry from log file */
        if (update_reg(reg_key, log_file) < 0) {
                perror("guru meditation");
                exit(1);
        }

        /* seek to end of log file in anticipation of writing */
        fseek(log_file, 0, SEEK_END);

        for (int i; i < MAX_BOXES; ++i) {
                printf("team %u has points %u!\n", reg_key[i].team_number, reg_key[i].uptime);
        }
        
        /* spawn socket and prepare for binding */
        if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                perror("guru meditation");
                exit(1);
        }
        
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
                perror("guru meditation");
                exit(1);
        }
        
        memset(&server_addr, 0, sizeof(struct sockaddr_in));    // zero out to prevent bad voodoo
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(PORT);
        
        /* bind socket and listen */
        if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof server_addr) < 0) {
                perror("guru meditation");
                exit(1);
        }
        
        if (listen(server_socket, 64) < 0) {
                perror("guru meditation");
                close(server_socket);
                exit(1);
        }
        
        /* set up epoll */
        if ((epoll_fd = epoll_create(MAX_CLIENTS + 1)) < 0) {
                perror("guru meditation");
                close(server_socket);
                exit(1);
        }
        
        /* add listening socket to epoll interest list */
        listener_event.events = EPOLLIN;
        listener_event.data.fd = server_socket;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &listener_event) < 0) {
                perror("guru meditation");
                close(server_socket);
                close(epoll_fd);
                exit(1);
        }

        /* main program loop - verify and handle clients by activity */
        for (;;) {
                /* block until some sockets are ready */
                if ((number_fds = epoll_wait(epoll_fd, ready_sockets, MAX_CLIENTS + 1, -1)) < 0) {
                        perror("guru meditation");
                        continue;
                }
                
                /* loop through ready sockets */
                for (int i = 0; i < number_fds; ++i) {
                        /* a new client is trying to connect */
                        if ((tmp_fd = ready_sockets[i].data.fd) == server_socket) {
                                if ((tmp_fd = accept_client(server_socket, epoll_fd)) < 0) {
                                        perror("guru meditation");
                                        continue;       // if accepting fails try the next ready socket
                                }
                        }
                        
                        /* receiving data from a client */
                        else {
                                switch (handle_client(tmp_fd, reg_key, log_file)) {
				case -2:        // attempt to register an already-registered client or authentification fails
					close(tmp_fd);

					break;
				case -1:        // read on socket fails (client disconnected), or too short inc interval, or something just doesn't add up
					/* if a team box, remove client from registry */
					for (int i = 0; i < MAX_BOXES; ++i) {
						if (reg_key[i].fd == tmp_fd) {
							close(tmp_fd);
							reg_key[i].fd = 0;
							break;
						} else {
							close(tmp_fd);
						}
					}

					break;
				case 0:
					// nothing to do ...
					break;
				default:
					fprintf(stderr, "if you are reading this, you messed up big time\n");
                                }
                        }
                }
        }
}
