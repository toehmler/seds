#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "seds-lib/seds-lib.h"

#define THREAD_POOL_SIZE 24 
#define QUEUE_SIZE 64 

pthread_t thread_pool[THREAD_POOL_SIZE];
rbuf *worker_queues[THREAD_POOL_SIZE];

int send_all(int s, char *buf, int *len);
void *worker_thread_loop(void *queue_ptr);
void handle_connection(struct conn *client);

int main(int argc, char *argv[]) {
    int client_fd, current_worker;
    int ids[THREAD_POOL_SIZE];
    struct serv server;
    /* accept and parse command line arguments */
    serv_init(&server, argc, argv);
    /* signal handle to prevent failure on broken pipe error */
    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
    /* initialize worker queues */
    for (int i=0; i < THREAD_POOL_SIZE; i++) {
        if ((worker_queues[i] = rbuf_create(QUEUE_SIZE)) == NULL) {
            fprintf(stderr, "Failed to initialize worker queues; exiting.\n");
            exit(1);
        }
    }
    /* launch thread pool */
    for (int i=0; i < THREAD_POOL_SIZE; i++) { ids[i] = i; }
    for (int i=0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, worker_thread_loop, &ids[i]);
    }
    /* setup server using networking code from library */
    if (serv_start(&server) < 0) {
        fprintf(stderr, "Server failed to start; exiting.\n");
        exit(1);
    }
    /* handle connections in a loop */
    current_worker = 0;
    while(1) {
        if ((client_fd = net_accept_conn(server.fd)) < 0) {
            perror("serv_accept");
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
        client->res.head_size = 0;
        client->res.size = 0;
        client->file.path = NULL;
        client->file.body = NULL;
        rbuf_write(worker_queues[current_worker], client);
        current_worker = (current_worker+1) % THREAD_POOL_SIZE;
    }
}

void *worker_thread_loop(void *queue_ptr) {
    int id = *((int *)queue_ptr);
    while(1) {
        struct conn *client;
        client = rbuf_read(worker_queues[id]);
        handle_connection(client);
    }
}


void handle_connection(struct conn *client) {
    /* read request */
    if (msg_req_read(client->fd, &client->req) <= 0) {
        goto cleanup;
    }

    /* parse request */
    if (msg_req_parse(&client->req) < 0) {
        goto cleanup;
    }

    /* perform file i/o */
    if (sfs_set_file(client->req.uri, &client->file, &client->code) < 0) {
        goto cleanup;
    }

    /* construct response */
    if (msg_res_build(&client->res, client->code, &client->file) < 0) {
        goto cleanup;
    }

    /* send response */
    if (send_all(client->fd, client->res.buf, &client->res.size) < 0) {
        goto cleanup;
    }

    goto cleanup;


cleanup:
    /* TODO unmap file and close fd */
        close(client->fd);
        if (client->file.path != NULL) {
            free(client->file.path);
        } 
        if (client->file.body != NULL) {
            free(client->file.body);
        }
        if (client->res.buf != NULL) {
            free(client->res.buf);
        }
        free(client);
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



