#include <stdio.h>
#include <stdlib.h>
#include "data.h"

net_msg build_msg(unsigned int op, unsigned int id)
{
        net_msg new_msg;
        
        new_msg.op = op;
        new_msg.id = id;
        new_msg.check = (op + id) ^ 'L';

        return new_msg;
}

char *stringify_msg(net_msg new_msg)
{
        /* message string should never exceed 12 bytes
           2 bytes + delimiter + 2 bytes + delimiter + 3 bytes + end-marker = 10 bytes
           extra room is left just in case */
        char *msg_string = malloc(12);
        snprintf(msg_string, 12, "%d+%d=%d?", new_msg.op, new_msg.id, new_msg.check);
        
        return msg_string;
}

net_msg destring_msg(char *new_msg)
{
        net_msg destringed_msg;

        sscanf(new_msg, "%d+%d=%d", &destringed_msg.op, &destringed_msg.id, &destringed_msg.check);
        free(new_msg);
        return destringed_msg;
}
