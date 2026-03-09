#ifndef MTHELPERS_H
#define MTHELPERS_H

#include "protocol.h"
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>

ssize_t read_full(int fd, void *buf, size_t count);
ssize_t write_full(int fd, const void *buf, size_t count);

void sigint_handler(int sig);
void sigusr1_handler(int sig);

void update_top3(uint64_t total);
void print_charities_stdout(void);
void print_stats_stderr(void);

#endif