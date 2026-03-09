#include "server.h"
#include "protocol.h"
#include "MThelpers.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

extern int clientCnt;
extern uint64_t maxDonations[3];
extern charity_t charities[5];

extern pthread_mutex_t stats_lock;
extern pthread_mutex_t charity_locks[5];

extern volatile sig_atomic_t sigint_flag;

ssize_t read_full(int fd, void *buf, size_t count) {
    size_t total = 0;
    char *ptr = buf;

    while (total < count) {
        ssize_t n = read(fd, ptr + total, count - total);
        if (n == 0) {
            return total;   // peer closed connection
        }
        if (n < 0) {
            if (errno == EINTR) {
                return -1;
            }
            return -1;
        }
        total += n;
    }
    return total;
}

ssize_t write_full(int fd, const void *buf, size_t count) {
    size_t total = 0;
    const char *ptr = buf;

    while (total < count) {
        ssize_t n = write(fd, ptr + total, count - total);
        if (n < 0) {
            if (errno == EINTR) {
                return -1;
            }
            return -1;
        }
        total += n;
    }
    return total;
}

void sigint_handler(int sig) {
    (void)sig;
    sigint_flag = 1;
}

void sigusr1_handler(int sig) {
    (void)sig;
}

void update_top3(uint64_t total) {
    pthread_mutex_lock(&stats_lock);

    if (total > maxDonations[0]) {
        maxDonations[2] = maxDonations[1];
        maxDonations[1] = maxDonations[0];
        maxDonations[0] = total;
    } else if (total > maxDonations[1]) {
        maxDonations[2] = maxDonations[1];
        maxDonations[1] = total;
    } else if (total > maxDonations[2]) {
        maxDonations[2] = total;
    }

    pthread_mutex_unlock(&stats_lock);
}

void print_charities_stdout(void) {
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&charity_locks[i]);
        printf("%d, %u, %lu, %lu\n",
               i,
               charities[i].numDonations,
               charities[i].topDonation,
               charities[i].totalDonationAmt);
        pthread_mutex_unlock(&charity_locks[i]);
    }
}

void print_stats_stderr(void) {
    pthread_mutex_lock(&stats_lock);
    fprintf(stderr, "%d\n", clientCnt);
    fprintf(stderr, "%lu, %lu, %lu\n",
            maxDonations[0], maxDonations[1], maxDonations[2]);
    pthread_mutex_unlock(&stats_lock);
}