/* 
 * queue.c
 */

#include <stdlib.h>
#include <stdio.h>
#include "inc/queue.h"

/* enqueue: places an item into a queue
 * @param: q ptr to a queue struct being added to
 * @param: data void ptr to the data being added to the queue
 * @returns: ptr to original data on success, NULL on err */
void *enqueue(queue *q, void *data) {
    /* initialize a new node */
    struct node_q *node;
    if ((node = malloc(sizeof(struct node_q))) == NULL) {
        return NULL;
    }
    node->data = data;
    /* add node to the queue */
    if (q->size > 0) {
        q->tail->next = node;
    } else {
        q->head = node;
    }
    node->next = NULL;
    q->tail = node;
    q->size++;
    return data;
}

/* dequeue: pulls and item from the queue
 * @param: a ptr to a queue struct 
 * @returns: ptr to data on success, NULL if empty */
void *dequeue(queue *q) {
    if (q->size == 0) {
        return NULL;
    } else {
        void *data = q->head->data;
        struct node_q *node = q->head;
        q->head = q->head->next;
        if (q->head == NULL) {q->tail = NULL;}
        free(node);
        q->size--;
        return data;
    }
}

/* assumes ints */
void print_queue(queue *q) {
    struct node_q *current = q->head;
    int *tmp;
    while (current != NULL) {
        tmp = (int *)current->data;
        printf("%d->", *tmp);
        current = current->next;
    }
    printf("\n");
}

