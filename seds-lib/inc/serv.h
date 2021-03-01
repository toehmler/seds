#ifndef __SERV_H
#define __SERV_H

#include "msg.h"
#include "sfs.h"

#define DEF_PORT "1024"
#define DEF_BACKLOG 10

struct serv {
    int fd;
    int backlog;
    char *port;
};

struct conn {
    int fd;
    int code;
    struct serv *server;
    struct req_msg req;
    struct res_msg res;
    struct sfile file;
};

void serv_init(struct serv *server, int argc, char *argv[]);
int serv_start(struct serv *server);

#endif
