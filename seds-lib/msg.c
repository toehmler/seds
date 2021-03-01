/* 
 * msg.c
 *
 * Used to deal with reading requests and sending responses
 * Will be a combination of request.c / response.c functionality 
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "inc/msg.h"
#include "inc/queue.h"
#include "inc/frmt.h"

/* msg_req_read: reads a socket and fills the buf of a req_msg
 * @param: fd the file des to read from
 * @param: req the req_msg whose buffer will be filled 
 * @returns: bytes read into buffer, -1 on err */
int msg_req_read(int fd, struct req_msg *req) {
    int bytes_recv = 0;
    if ((bytes_recv = recv(fd, req->buf, BUF_SIZE, 0)) < 0) {
        perror("recv");
        printf("recv err\n");
        return -1;
    }
    return bytes_recv;
}

/* request_parse: parses the buffer of a given request 
 * @param: req a pointer to a request struct
 * @returns: 0 on success, -1 on err */
int msg_req_parse(struct req_msg *req) {
    /* strip just the first line */
    char *line = strtok(req->buf, "\n");
    if ((req->method = strtok(line, " ")) == NULL) {
        printf("method err\n");
        return -1;
    }
    if ((req->uri = strtok(NULL, " ")) == NULL) {
        printf("uri err\n");
        return -1;
    }
    if ((req->version = strtok(NULL, " ")) == NULL) {
        printf("version err\n");
        return -1;
    }
//    req->version = NULL;
    return 0;
}

/* msg_res_build: constructs formatted http response
 * @param: res ptr to res_msg struct that will hold response
 * @param: code the response code to include
 * @param: file a ptr to sfile struct to set as the body of response 
 * @returns: 0 on success, -1 on err */
int msg_res_build(struct res_msg *res, int code, struct sfile *file) {
    if ((res->head_size = msg_res_gen_hdrs(res, code)) < 0) {
        return -1;
    }
    if (file != NULL) {
        int size;
        if ((size = msg_res_file_hdrs(res, file)) < 0) {
            return -1;
        }
        res->head_size += size;
        if ((size = msg_res_body(res, file)) < 0) {
            return -1;
        }
        res->size += size;
    } else {
        res->size = res->head_size;
        res->buf = res->head_buf;
    } 
    return 0;
}
/* msg_res_gen_hdrs: sets the gen headers of a response (msg, server, date)
 * @param: res a pointer to a response struct
 * @param: code an int used to get the response message
 * @returns: size in bytes of headers, -1 on err */
int msg_res_gen_hdrs(struct res_msg *res, int code) {
    int total = 0, line_len = 0;
    int remain = BUF_SIZE;
    char *ptr = res->head_buf;
    /* set the resline (code, message, version) */
    if ((line_len = frmt_set_resline(ptr, remain, VERSION, code)) < 0) {
        return -1;
    }
    total += line_len;
    remain -= line_len;
    ptr += line_len;
    /* set the server name */
    if ((line_len = frmt_set_servline(ptr, remain, SERVER)) < 0) {
        return -1;
    }
    total += line_len;
    remain -= line_len;
    ptr += line_len;
    /* set date line */
    time_t now = time(0);
    if ((line_len = frmt_set_date(ptr, remain, &now, "Date")) < 0) {
        return -1;
    }
    return total += line_len;
}

/* msg_res_file_hdrs: sets the files headers of a response (last-mod, type, len) 
 * @note: leaves file memory-mapped
 * @param: res a pointer to a response struct
 * @param: file a pointer to a bfile struct
 * @returns: size in bytes of file headers */
int msg_res_file_hdrs(struct res_msg *res, struct sfile *file) {
    int total = 0, line_len = 0;
    int remain = BUF_SIZE - res->head_size;
    char *ptr = res->head_buf + res->head_size;
    /* set connection line */
    if ((line_len = snprintf(ptr, remain, "Connection: Keep-Alive\r\n")) < 0) {
        return -1;
    }
    total += line_len;
    remain -= line_len;
    ptr += line_len;
    /* set the content length */
    if ((line_len = frmt_set_cont_len(ptr, remain, file->size)) < 0) {
        return -1;
    }
    total += line_len;
    remain -= line_len;
    ptr += line_len;
    /* set the content type */
    if ((line_len = frmt_set_date(ptr, remain, &file->last_mod, "Last-Modified")) < 0) {
        return -1;
    }
    total += line_len;
    remain -= line_len;
    ptr += line_len;
    /* set the last modified */
    if ((line_len = frmt_set_cont_type(ptr, remain, FILETYPE)) < 0) {
        return -1;
    }
    return total += line_len;
}

/* msg_res_body: sets the body of a response
 * @param: res a pointer to a response struct
 * @param: file a pointer to a bfile struct
 * @returns: bytes written to buffer, -1 on err */
int msg_res_body(struct res_msg *res, struct sfile *file) {
    /* allocate memory for headers and body */
    int total_size;
    total_size = res->head_size + file->size;
    if ((res->buf = malloc(sizeof(char)*total_size)) == NULL)
        return -1;
    /* copy headers and body into buffer */
    memcpy(res->buf, res->head_buf, res->head_size);
    memcpy(res->buf + res->head_size, file->body, file->size);
    return total_size;
}






