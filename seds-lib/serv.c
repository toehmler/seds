/* 
 * serv.c
 * Functionality related to high-level server operations (start, stop, etc)
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "inc/serv.h"
#include "inc/net.h"

/* init_serv: parses args and sets fields of serv struct 
 * @param: server a ser struct to populate
 * @param: argc the argument count
 * @param: argv the arguments to parse
 * @note: exits on err parsing flags */
void serv_init(struct serv *server, int argc, char *argv[]) {
    int flag;
    server->port = DEF_PORT;
    server->backlog = DEF_BACKLOG;
    server->fd = -1; /* will be set for real by server_start() */
    while ((flag = getopt(argc, argv, "p:b:")) != -1) {
        switch(flag) {
            case 'p':
                server->port = optarg;
                break;
            case 'b':
                server->backlog = atoi(optarg);
            default:
                fprintf(stderr, "Usage: %s [-p port] [-b backlog]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

/* serv_start: see net.c/net_get_socket() */
int serv_start(struct serv *server) {
    if ((server->fd = net_get_socket(server->port, server->backlog)) < 0) {
        return -1;
    } else {
        return 0;
    }
}





