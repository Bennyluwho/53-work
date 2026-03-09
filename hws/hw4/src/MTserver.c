#include "server.h"
#include "protocol.h"
#include "MThelpers.h"
#include "linkedlist.h"

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

/**********************DECLARE ALL LOCKS HERE BETWEEN THES LINES FOR MANUAL GRADING*************/
pthread_mutex_t stats_lock;
pthread_mutex_t charity_locks[5];
pthread_mutex_t log_lock;
pthread_mutex_t thread_list_lock;
/***********************************************************************************************/


// Global variables, statistics collected since server start-up
int clientCnt;  // # of client connections made, Updated by the main thread
uint64_t maxDonations[3];  // 3 highest total donations amounts (sum of all donations to all
                           // charities in one connection), updated by client threads
                           // index 0 is the highest total donation
charity_t charities[5]; // Global variable, one charity per index
volatile sig_atomic_t sigint_flag = 0;

FILE *log_fp = NULL;
thread_list_t client_threads;

typedef struct {
    int client_fd;
} client_arg_t;

//logs actions made (mainly used for debugging because I seem to be a chud)
static void log_action(const char *fmt, ...) {
    va_list args;
    pthread_mutex_lock(&log_lock);
    va_start(args, fmt);
    vfprintf(log_fp, fmt, args);
    fflush(log_fp);
    va_end(args);
    pthread_mutex_unlock(&log_lock);
}

//client thread function
//it allows one thread per cient
//thread repeatedly until  message is read
//on exit it also updates the maxDonations
void *client_thread_main(void *arg) {
    client_arg_t *carg = (client_arg_t *)arg;
    int fd = carg->client_fd;
    free(carg);

    uint64_t local_total = 0;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigusr1_handler;
    sigaction(SIGUSR1, &sa, NULL);

    while (1) {
        message_t *msg = calloc(1, sizeof(message_t));
        if (msg == NULL) {
            break;
        }

        ssize_t n = read_full(fd, msg, sizeof(message_t));
        if (n == 0) {
            free(msg);
            break;
        }
        if (n < 0) {
            free(msg);
            if (sigint_flag) {
                break;
            }
            break;
        }

        if (msg->msgtype == DONATE) {
            uint8_t charity = msg->msgdata.donation.charity;
            uint64_t amount = msg->msgdata.donation.amount;

            if (charity >= 5 || amount == 0) {
                msg->msgtype = ERROR;
                write_full(fd, msg, sizeof(message_t));
                log_action("%d ERROR\n", fd);
                free(msg);
                continue;
            }
            
            //ensures that the charity donation amount is properly updated and is 
            //touched by one thread at a time.
            pthread_mutex_lock(&charity_locks[charity]);
            charities[charity].totalDonationAmt += amount;
            charities[charity].numDonations += 1;
            if (amount > charities[charity].topDonation) {
                charities[charity].topDonation = amount;
            }
            pthread_mutex_unlock(&charity_locks[charity]);

            local_total += amount;

            write_full(fd, msg, sizeof(message_t));
            log_action("%d DONATE %u %lu\n", fd, charity, amount);
        }
        else if (msg->msgtype == CINFO) {
            uint8_t charity = msg->msgdata.donation.charity;

            if (charity >= 5) {
                msg->msgtype = ERROR;
                write_full(fd, msg, sizeof(message_t));
                log_action("%d ERROR\n", fd);
                free(msg);
                continue;
            }

            pthread_mutex_lock(&charity_locks[charity]);
            charity_t info = charities[charity];
            pthread_mutex_unlock(&charity_locks[charity]);

            memset(msg, 0, sizeof(message_t));
            msg->msgtype = CINFO;
            msg->msgdata.charityInfo = info;

            write_full(fd, msg, sizeof(message_t));
            log_action("%d CINFO %u\n", fd, charity);
        }
        else if (msg->msgtype == TOP) {
            pthread_mutex_lock(&stats_lock);
            uint64_t top0 = maxDonations[0];
            uint64_t top1 = maxDonations[1];
            uint64_t top2 = maxDonations[2];
            pthread_mutex_unlock(&stats_lock);

            memset(msg, 0, sizeof(message_t));
            msg->msgtype = TOP;
            msg->msgdata.maxDonations[0] = top0;
            msg->msgdata.maxDonations[1] = top1;
            msg->msgdata.maxDonations[2] = top2;

            write_full(fd, msg, sizeof(message_t));
            log_action("%d TOP\n", fd);
        }
        else if (msg->msgtype == LOGOUT) {
            log_action("%d LOGOUT\n", fd);
            free(msg);
            break;
        }
        else {
            msg->msgtype = ERROR;
            write_full(fd, msg, sizeof(message_t));
            log_action("%d ERROR\n", fd);
        }

        free(msg);

        if (sigint_flag) {
            break;
        }
    }

    if (local_total > 0) {
        update_top3(local_total);
    }

    pthread_mutex_lock(&thread_list_lock);
    thread_list_remove(&client_threads, pthread_self());
    pthread_mutex_unlock(&thread_list_lock);

    close(fd);
    return NULL;
}

int main(int argc, char *argv[]) {

    // Arg parsing
    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stderr, USAGE_MSG_MT);
                exit(EXIT_FAILURE);
        }
    }

    // 3 positional arguments necessary
    if (argc - optind != 2) {
        fprintf(stderr, USAGE_MSG_MT);
        exit(EXIT_FAILURE);
    }
    unsigned int port_number = atoi(argv[optind]);
    char *log_filename = argv[optind + 1];

    // INSERT SERVER INITIALIZATION CODE HERE
    clientCnt = 0;
    for (int i = 0; i < 3; i++) {
        maxDonations[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        charities[i].totalDonationAmt = 0;
        charities[i].topDonation = 0;
        charities[i].numDonations = 0;
    }

    if (pthread_mutex_init(&stats_lock, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&log_lock, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 5; i++) {
        if (pthread_mutex_init(&charity_locks[i], NULL) != 0) {
            exit(EXIT_FAILURE);
        }
    }
    if (pthread_mutex_init(&thread_list_lock, NULL) != 0) {
        exit(EXIT_FAILURE);
    }

    thread_list_init(&client_threads);

    log_fp = fopen(log_filename, "w");
    if (log_fp == NULL) {
        exit(2);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        exit(EXIT_FAILURE);
    }

    // Initiate server socket for listening
    int listen_fd = socket_listen_init(port_number);
    printf("Currently listening on port: %d.\n", port_number);
    int client_fd;
    struct sockaddr_in client_addr;
    unsigned int client_addr_len = sizeof(client_addr);

    while(1) {
        // Wait and Accept the connection from client
        client_fd = accept(listen_fd, (SA*)&client_addr, &client_addr_len);

        // INSERT SERVER ACTIONS FOR CONNECTED CLIENT CODE HERE

        //check to ensure connection was succesfully made
        if (client_fd < 0) {
            if (errno == EINTR) {
                if (sigint_flag) {
                    break;
                }
                continue;
            }
            perror("accept");
            exit(EXIT_FAILURE);
        }

        client_arg_t *arg = malloc(sizeof(client_arg_t));
        if (arg == NULL) {
            close(client_fd);
            continue;
        }

        arg->client_fd = client_fd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread_main, arg) != 0) {
            free(arg);
            close(client_fd);
            continue;
        }
        
        pthread_detach(tid);

        pthread_mutex_lock(&thread_list_lock);
        thread_list_push_back(&client_threads, tid);
        pthread_mutex_unlock(&thread_list_lock);

        //IMPORTANT: ensures that client count per thread is properly counted.
        //thank goodness for mutex locks :P
        pthread_mutex_lock(&stats_lock);
        clientCnt++;
        pthread_mutex_unlock(&stats_lock);

        if (sigint_flag) {
            break;
        }
    }

    close(listen_fd);

    //Proper shutdown logic which ensures that stats are printed and 
    //that threads are not doing anything silly....
    pthread_mutex_lock(&thread_list_lock);
    thread_node_t *curr = client_threads.head;
    while (curr != NULL) {
        pthread_kill(curr->tid, SIGUSR1);
        curr = curr->next;
    }
    pthread_mutex_unlock(&thread_list_lock);

    print_charities_stdout();
    print_stats_stderr();

    thread_list_destroy(&client_threads);

    fclose(log_fp);

    pthread_mutex_destroy(&thread_list_lock);
    pthread_mutex_destroy(&stats_lock);
    pthread_mutex_destroy(&log_lock);
    for (int i = 0; i < 5; i++) {
        pthread_mutex_destroy(&charity_locks[i]);
    }

    return 0;
}

int socket_listen_init(int server_port){
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully created\n");

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(server_port);

    int opt = 1;

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt))<0)
    {
    	perror("setsockopt");exit(EXIT_FAILURE);
    }

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully binded\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 1)) != 0) {
        printf("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}


