/* 
 * simple-serv.c
 *
 * A very basic, single threaded servert that handles clients sequentially.
 * I/O operations (ie. send(2) and recv(2)) block
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include "seds-lib/seds-lib.h"

#define THREAD_POOL_SIZE 24 
#define CHUNK 4096 


pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cv = PTHREAD_COND_INITIALIZER;

int send_all(int s, char *buf, int *len);
void *worker_thread_loop(void *queue_ptr);
void handle_connection(struct conn *client);
int send_chunked(int fd, char *buf, int len);

int main(int argc, char *argv[]) {
    int client_fd;
    struct serv server;
    /* accept and parse command line arguments */
    serv_init(&server, argc, argv);
    /* signal handle to prevent failure on broken pipe error */
    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
    /* setup server using networking code from library */
    if (serv_start(&server) < 0) {
        fprintf(stderr, "Server failed to start; exiting.\n");
        exit(1);
    }
    /* create queue & thread pool */ 
    queue work_queue;
    work_queue.head = NULL;
    work_queue.tail = NULL;
    work_queue.size = 0;
    for (int i=0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, worker_thread_loop, &work_queue);
    }
    /* handle connections in a loop */
    while(1) {
        if ((client_fd = net_accept_conn(server.fd)) < 0) {
            perror("serv_accept");
            break;
        }
        struct conn *client;
        if ((client = malloc(sizeof(struct conn))) == NULL) {
            perror("malloc");
            exit(1);
        }
        client->fd = client_fd;
        client->server = &server;
        client->res.buf = NULL;
        client->res.size = 0;
        client->res.head_size = 0;
        client->file.path = NULL;
        client->file.body = NULL;
        /* lock mutex and enqueue client */
        pthread_mutex_lock(&queue_mtx);
        enqueue(&work_queue, client);
        pthread_cond_signal(&queue_cv);
        pthread_mutex_unlock(&queue_mtx);
    }
    /* TODO: dont forget about signal handlers */
}

/* worker_thread_loop: routine performed by worker threads in pool 
 * @note: relies on global mutexes and condition vars
 * @param: queue_ptr void ptr to a queue with conn structs */
void *worker_thread_loop(void *queue_ptr) {
    queue *work_queue = (queue *)queue_ptr;
    while(1) {
        struct conn *client;
        pthread_mutex_lock(&queue_mtx);
        pthread_cond_wait(&queue_cv, &queue_mtx);
        client = dequeue(work_queue);
        pthread_mutex_unlock(&queue_mtx);
        if (client != NULL) {
            handle_connection(client);
        }
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

/* send_chunked: writes a buffer of a known size to a socket in chunks
 * @note: chunks are made as response is being sent 
 * @param: fd the socket to write to 
 * @param: buf a char ptr to a buffer that will be chunked and sent
 * @param: len total number of bytes to send
 * @returns: total number of bytes sent */
int send_chunked(int fd, char *buf, int len) {
    int bytes_left = len;
    int total_sent = 0, chunk_no = 0;
    int n, chunk_size;
    char *buf_ptr = buf;
    while (bytes_left > 0) {
        if (bytes_left > CHUNK)
            chunk_size = CHUNK;
        else
            chunk_size = bytes_left;
//        printf("sending chunk %d\n", chunk_no);
        n = send(fd, buf_ptr, chunk_size, 0);
        if (n == -1) { 
            perror("send");
            break; 
        }
        total_sent += n;
        bytes_left -= n;
        buf_ptr += n;
        chunk_no++;
    }
    printf("total bytes sent: %d\n", total_sent);
    return n == -1 ? -1 : total_sent;
}



