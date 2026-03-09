#include "linkedlist.h"
#include <stdlib.h>

//linked list initalizer
void thread_list_init(thread_list_t *list) {
    if (list == NULL) return;

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

//linked list insertion
int thread_list_push_back(thread_list_t *list, pthread_t tid) {
    if (list == NULL) return -1;

    thread_node_t *new_node = malloc(sizeof(thread_node_t));
    if (new_node == NULL) return -1;

    new_node->tid = tid;
    new_node->next = NULL;

    if (list->tail == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->length++;
    return 0;
}

//linked list removal which compares thread id to distingusish what to remove
int thread_list_remove(thread_list_t *list, pthread_t tid) {
    if (list == NULL || list->head == NULL) return -1;

    thread_node_t *prev = NULL;
    thread_node_t *curr = list->head;

    while (curr != NULL) {
        if (pthread_equal(curr->tid, tid)) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }

            if (list->tail == curr) {
                list->tail = prev;
            }

            free(curr);
            list->length--;
            return 0;
        }

        prev = curr;
        curr = curr->next;
    }

    return -1;
}

//linekd list destroyer
void thread_list_destroy(thread_list_t *list) {
    if (list == NULL) return;

    thread_node_t *curr = list->head;
    while (curr != NULL) {
        thread_node_t *next = curr->next;
        free(curr);
        curr = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}