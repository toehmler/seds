#ifndef __RBUF_H
#define __RBUF_H

#include <pthread.h>

typedef struct {
    void **data_buf;
    int write_idx, read_idx;
    int size;
    int full, empty;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} rbuf;

rbuf *rbuf_create(int size);
void rbuf_write(rbuf *buf, void *data);
void *rbuf_read(rbuf *buf);

void rbuf_print(rbuf *buf);





#endif
