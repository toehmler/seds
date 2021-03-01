#ifndef __QUEUE_H
#define __QUEUE_H

struct node_q {
    void *data;
    struct node_q *next;
};

typedef struct {
    struct node_q *head;
    struct node_q*tail;
    int size;

} queue;

void *enqueue(queue *q, void *data);
void *dequeue(queue *q);

void print_queue(queue *q);

#endif
