#define BUFSIZE 12

/* message structure for client -> server comms */
typedef struct {
        unsigned int op;        // 47 for auth, 73 for inc
        unsigned int id;        // this is the team id (10-21 for chctf-1)
        unsigned int check;     // simple checksum; check = (op + id) ^ 'L'
} net_msg;

/* load operation and id into the message structure.
   takes an operator and team id and returns a whole net_msg struct. */
net_msg build_msg(unsigned int op, unsigned int id) {
        net_msg new_msg;
        
        new_msg.op = op;
        new_msg.id = id;
        new_msg.check = (op + id) ^ 'L';

        return new_msg;
}

/* convert net_msg structure into ANSI string for transfer.
   takes a net_msg struct and returns a char pointer of 12 bytes. */
char *stringify_msg(net_msg new_msg) {
        /* message string should never exceed 12 bytes
           2 bytes + delimiter + 2 bytes + delimiter + 3 bytes + end-marker = 10 bytes
           extra room is left just in case */
        char *msg_string = malloc(12);
        snprintf(msg_string, 12, "%d+%d=%d?", new_msg.op, new_msg.id, new_msg.check);
        
        return msg_string;
}

/* convert a stringified net_msg back to a net_msg structure.
   takes a pointer to a string and returns a net_msg structure.
   this function frees the pointer passed to it. */
net_msg destring_msg(char *new_msg) {
        net_msg destringed_msg;

        sscanf(new_msg, "%d+%d=%d", &destringed_msg.op, &destringed_msg.id, &destringed_msg.check);
        free(new_msg);
        return destringed_msg;
}