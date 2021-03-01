/* 
 * rbuf.c
 *
 * Provides interface for using ring buffers (single-reader, single-writer queues)
 * Has the advantage of less lock contention and less required synchronization
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "inc/rbuf.h"

/* rbuf_create: initializes a ring buffer of a given size
 * @param: size how many slots should be allocated in the buffer
 * @returns: ptr to the buffer on success, NULL on err */
rbuf *rbuf_create(int size) {
    rbuf *new_buf;
    if ((new_buf = malloc(sizeof(rbuf))) == NULL) {
        perror("malloc");
        return NULL;
    }
    memset(new_buf, 0, sizeof(rbuf)); 
    if ((new_buf->data_buf = malloc(size * sizeof(void *))) == NULL) {
        perror("malloc");
        free(new_buf);
        return NULL;
    }
    if (pthread_mutex_init(&new_buf->mtx, NULL) < 0) {
        perror("pthread_mutex_init");
        free(new_buf);
        free(new_buf->data_buf);
        return NULL;
    }
    if (pthread_cond_init(&new_buf->cv, NULL) < 0) {
        perror("pthread_cond_init");
        free(new_buf);
        free(new_buf->data_buf);
        return NULL;
    }
    new_buf->size = size;
    new_buf->full = 0;
    new_buf->empty = 1;
    new_buf->write_idx = 0;
    new_buf->read_idx = 0;
    return new_buf;
}

/* rbuf_write: inserts data into a given buffer
 * @param: buf a ptr to a ring buffer
 * @param: data void ptr to data that will be written to the buffer */
void rbuf_write(rbuf *buf, void *data) {
    /* if full wait for data to be read */
    if (buf->full == 1) {
        pthread_mutex_lock(&buf->mtx);
        pthread_cond_wait(&buf->cv, &buf->mtx);
        pthread_mutex_unlock(&buf->mtx);
    }
    /* store data increment write index */
    buf->data_buf[buf->write_idx] = data;
    buf->write_idx = (buf->write_idx+1) % buf->size;
    /* signal to reader if waiting */
    if (buf->empty == 1) {
        pthread_mutex_lock(&buf->mtx);
        buf->empty = 0;
        pthread_cond_signal(&buf->cv);
        pthread_mutex_unlock(&buf->mtx);
    } else if (buf->write_idx == buf->read_idx) {
        /* set flag if buffer full */
        buf->full = 1;
    }
}

/* rbuf_read: reads data from a given buffer
 * @note: effectively removes data from buffer (but doesn't free it)
 * @param: buf the ring buffer to read data from
 * @returns: ptr to data, NULL if buffer is empty */
void *rbuf_read(rbuf *buf) {
    /* if empty wait for data to be written */
    if (buf->empty == 1) {
        pthread_mutex_lock(&buf->mtx);
        pthread_cond_wait(&buf->cv, &buf->mtx);
        pthread_mutex_unlock(&buf->mtx);
    }
    /* store ptr to data and increment read index */
    void *data = buf->data_buf[buf->read_idx];
    buf->read_idx = (buf->read_idx+1) % buf->size;
    /* signal to writer if waiting */
    if (buf->full == 1) {
        pthread_mutex_lock(&buf->mtx);
        buf->full = 0;
        pthread_cond_signal(&buf->cv);
        pthread_mutex_unlock(&buf->mtx);
    } else if (buf->write_idx == buf->read_idx) {
        /* set flag if buffer empty */
        buf->empty = 1;
    }
    return data;
}

/* print_rbuf: shows contents of a ring buffer (for debugging purposes 
 * @note: assumes char * being stored in buffer */
void rbuf_print(rbuf *buf) {
    int i = buf->read_idx;
    while (i != buf->write_idx) {
        char *data = buf->data_buf[i];
        printf("(%d): %s\n", i, data);
        i = (i+1) % buf->size;
    }
}
