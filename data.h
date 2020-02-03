/* message structure for client -> server comms */
typedef struct {
        unsigned int op;        // 47 for auth, 73 for inc
        unsigned int id;        // this is the team id (10-21 for chctf-1)
        unsigned char *addr;    // client ip address
        unsigned char *data;    // any extra data; only a simple checksum in current implementation
} net_msg;

net_msg build_msg(unsigned int op, unsigned int id, unsigned char *addr) {
        net_msg new_msg;
        
        new_msg.op = op;
        new_msg.id = id;
        new_msg.addr = 
}