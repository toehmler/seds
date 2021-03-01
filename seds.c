/* 
 * seds.c
 *
 * Proof-of-concept for a SEDA (staged event-driven architecture) server
 * New connections are handled in stages with each stage
 * Each stage is handled by a single thread and 
 * Stages are linked through ring buffers, a stage recieves work from its own 
 * buffer and places work in the ring buffer of the next stage */

/* reader */
/* parser */
/* file i/o */
/* builder */
/* sender */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "seds-lib/seds-lib.h"

#define NUM_STAGES 5
#define STAGE_BUF_SIZE 10

int send_all(int s, char *buf, int *len);
void *request_reader(void *arg);
void *request_parser(void *arg);
void *file_handler(void *arg);
void *response_builder(void *arg);
void *response_sender(void *arg);

rbuf *stage_bufs[NUM_STAGES];

int main(int argc, char *argv[]) {
    struct serv server;
    int ids[NUM_STAGES];
    int client_fd;
    pthread_t stage_threads[NUM_STAGES];
    /* accept and parse command line arguments */
    serv_init(&server, argc, argv);
    /* signal handle to prevent failure on broken pipe error */
    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
    /* initialize stage buffers */
    for (int i=0; i < NUM_STAGES; i++) {
        if ((stage_bufs[i] = rbuf_create(STAGE_BUF_SIZE)) == NULL) {
            fprintf(stderr, "Failed to initialize stage buffers; exiting.\n");
            exit(1);
        }
    }
    /* launch thread for each stage */
    for (int i=0; i < NUM_STAGES; i++) { ids[i] = i; }
    pthread_create(&stage_threads[0], NULL, request_reader, &ids[0]);
    pthread_create(&stage_threads[1], NULL, request_parser, &ids[1]);
    pthread_create(&stage_threads[2], NULL, file_handler, &ids[2]);
    pthread_create(&stage_threads[3], NULL, response_builder, &ids[3]);
    pthread_create(&stage_threads[4], NULL, response_sender, &ids[4]);
    /* setup server using networking code from library */
    if (serv_start(&server) < 0) {
        fprintf(stderr, "Server failed to start; exiting.\n");
        exit(1);
    }
    /* handle connections in a loop */
    while(1) {
        if ((client_fd = net_accept_conn(server.fd)) < 0) {
            break;
        }
        struct conn *client;
        if ((client = malloc(sizeof(struct conn))) == NULL) {
            perror("malloc");
            break;
        }
        client->fd = client_fd;
        client->server = &server;
        client->res.buf = NULL;
        client->res.size = 0;
        client->res.head_size = 0;
        client->file.path = NULL;
        client->file.body = NULL;
        rbuf_write(stage_bufs[0], client);
    }

    for (int i=0; i < NUM_STAGES; i++) {
        pthread_join(stage_threads[i], NULL);
    }
}


void *request_reader(void *arg) {
    int id = *((int *)arg);
    rbuf *buf = stage_bufs[id];
    rbuf *next_buf = stage_bufs[id+1];
    struct conn *client;
    while (1) {
        client = rbuf_read(buf);
        if (msg_req_read(client->fd, &client->req) > 0) {
            rbuf_write(next_buf, client);
        }
    }
    return NULL;
}
void *request_parser(void *arg) {
    int id = *((int *)arg);
    rbuf *buf = stage_bufs[id];
    rbuf *next_buf = stage_bufs[id+1];
    struct conn *client;
    while (1) {
        client = rbuf_read(buf);
        if (msg_req_parse(&client->req) == 0) {
            rbuf_write(next_buf, client);
        } 

    }
    return NULL;
}
void *file_handler(void *arg) {
    int id = *((int *)arg);
    rbuf *buf = stage_bufs[id];
    rbuf *next_buf = stage_bufs[id+1];
    struct conn *client;
    while (1) {
        client = rbuf_read(buf);
        if (sfs_set_file(client->req.uri, &client->file, &client->code) == 0) {
            rbuf_write(next_buf, client);
        }
    }
    return NULL;
}
void *response_builder(void *arg) {
    int id = *((int *)arg);
    rbuf *buf = stage_bufs[id];
    rbuf *next_buf = stage_bufs[id+1];
    struct conn *client;
    while (1) {
        client = rbuf_read(buf);
        if (msg_res_build(&client->res, client->code, &client->file) == 0) {
            rbuf_write(next_buf, client);
        }
    }
    return NULL;
}
void *response_sender(void *arg) {
    int id = *((int *)arg);
    rbuf *buf = stage_bufs[id];
    struct conn *client;
    while (1) {
        client = rbuf_read(buf);
        send_all(client->fd, client->res.buf, &client->res.size);
        /* TODO: free client */
        close(client->fd);
    }
    return NULL;
}

int send_all(int s, char *buf, int *len) {
    int total = 0;
    int bytes_left = *len;
    int n;
    while(total < *len) {
        n = write(s, buf+total, bytes_left);
        if (n == -1) { break; }
        total += n;
        bytes_left -= n;
    }
    *len = total; // store number of bytes actually sent 
    return n == -1 ? -1 : 0;
}



