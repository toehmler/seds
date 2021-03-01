#ifndef __MSG_H
#define __MSG_H

#include "sfs.h"

#define BUF_SIZE 512
#define VERSION "HTTP/1.1"
#define SERVER "Butler v0.01"
#define FILETYPE "image/jpeg"


struct req_msg {
    char *method;
    char *uri;
    char *version;
    char buf[BUF_SIZE];
};

struct res_msg {
    char head_buf[BUF_SIZE];
    int head_size;
    char *buf;
    int size;
};

int msg_req_read(int fd, struct req_msg *req);
int msg_req_parse(struct req_msg *req);
int msg_res_build(struct res_msg *res, int code, struct sfile *file);

int msg_res_gen_hdrs(struct res_msg *res, int code);
int msg_res_file_hdrs(struct res_msg *res, struct sfile *file);
int msg_res_body(struct res_msg *res, struct sfile *file);



#endif
