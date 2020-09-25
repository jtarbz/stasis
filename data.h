#ifndef DATA_H
#define DATA_H

#include <time.h>

#define BUFSIZE 12
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

/* message structure for client -> server comms */
typedef struct {
        unsigned int op;        // 47 for auth, 73 for inc
        unsigned int id;        // this is the team id (10-21 for chctf-1)
        unsigned int check;     // simple checksum; check = (op + id) ^ 'L'
} net_msg;

/* load operation and id into the message structure.
   takes an operator and team id and returns a whole net_msg struct. */
net_msg build_msg(unsigned int op, unsigned int id);

/* convert net_msg structure into ANSI string for transfer.
   takes a net_msg struct and returns a char pointer of 12 bytes. */
char *stringify_msg(net_msg new_msg);

/* convert a stringified net_msg back to a net_msg structure.
   takes a pointer to a string and returns a net_msg structure.
   this function frees the pointer passed to it. */
net_msg destring_msg(char *new_msg);

#endif
