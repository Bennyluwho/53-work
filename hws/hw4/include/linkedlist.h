#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <pthread.h>

typedef struct thread_node {
    pthread_t tid;
    int finished;
    struct thread_node *next;
} thread_node_t;

typedef struct {
    thread_node_t *head;
    thread_node_t *tail;
    int length;
} thread_list_t;

void thread_list_init(thread_list_t *list);
int thread_list_push_back(thread_list_t *list, pthread_t tid);
int thread_list_remove(thread_list_t *list, pthread_t tid);
void thread_list_destroy(thread_list_t *list);

#endif