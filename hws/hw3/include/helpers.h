#ifndef HELPERS_H
#define HELPERS_H

#define HIST_MAX 5

#include "icssh.h"
#include "linkedlist.h"
// A header file for helpers.c
// Declare any additional functions in this file

//background linked list functions
int bgentry_compare_seconds(const void *a, const void *b);
void bgentry_deleter(void *data);
void bgentry_printer(void *data, void *fp);
bgentry_t *remove_bgentry_by_pid(list_t *list, pid_t pid);

//history helper functions
void history_add(const char *line);
void history_print(void);
void history_cleanup(void);
char *history_get(int n);

void reap_bg(list_t *bg_list);
void remove_and_print_bgpid(list_t *bg_list, pid_t done);



#endif