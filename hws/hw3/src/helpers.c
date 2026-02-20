#include "helpers.h"
// Your helper functions need to be here.

static char *hist[HIST_MAX];
static int hist_count = 0;
static int hist_next = 0; 

int bgentry_compare_seconds(const void *a, const void *b) {
    const bgentry_t *ea = (const bgentry_t *)a;
    const bgentry_t *eb = (const bgentry_t *)b;

    if (ea->seconds < eb->seconds) return -1;
    if (ea->seconds > eb->seconds) return 1;

    //tie breaker
    if (ea->pid < eb->pid) return -1;
    if (ea->pid > eb->pid) return 1;
    return 0;
}

void bgentry_deleter(void *data) {
    bgentry_t *entry = (bgentry_t *)data;
    if (entry == NULL) return;

    free_job(entry->job);
    free(entry);
}

void bgentry_printer(void *data, void *fp) {
    bgentry_t *entry = (bgentry_t *)data;
    FILE *out = (FILE *)fp;

    fprintf(out, "pid=%d time=%ld cmd=%s\n",
            (int)entry->pid,
            (long)entry->seconds,
            entry->job && entry->job->line ? entry->job->line : "(null)");
}

bgentry_t *remove_bgentry_by_pid(list_t *list, pid_t pid) {
    if (list == NULL) return NULL;

    node_t *cur = list->head;
    int idx = 0;

    while (cur != NULL) {
        bgentry_t *entry = (bgentry_t *)cur->data;
        if (entry != NULL && entry->pid == pid) {
            return (bgentry_t *)RemoveByIndex(list, idx);
        }
        cur = cur->next;
        idx++;
    }
    return NULL; 
}

void history_add(const char *line) {
    if (!line || *line == '\0') return;

    free(hist[hist_next]);
    hist[hist_next] = strdup(line);

    hist_next = (hist_next + 1) % HIST_MAX;
    if (hist_count < HIST_MAX) hist_count++;
}

void history_print(void) {
    for (int i = 0; i < hist_count; i++) {
        int idx = (hist_next - 1 - i + HIST_MAX) % HIST_MAX;
        printf("%d: %s\n", i + 1, hist[idx]);
    }
}

void history_cleanup(void) {
    for (int i = 0; i < HIST_MAX; i++) {
        free(hist[i]);
        hist[i] = NULL;
    }
}

char *history_get(int n) {
    if (n < 1 || n > hist_count) return NULL;

    int idx = (hist_next - n + HIST_MAX) % HIST_MAX;
    return hist[idx];
}

void reap_bg(list_t *bg_list) {
    int status;
    pid_t done;

    while ((done = waitpid(-1, &status, WNOHANG)) > 0) {
        node_t *cur = bg_list->head;
        int idx = 0;

        while (cur != NULL) {
            bgentry_t *e = (bgentry_t *)cur->data;
            if (e != NULL && e->pid == done) {

                char *line_copy = strdup(e->job->line);

                bgentry_t *removed = (bgentry_t *)RemoveByIndex(bg_list, idx);
                bgentry_deleter(removed);
                bg_count--;

                printf(BG_TERM, (int)done, line_copy);
                free(line_copy);
                break;
            }
            cur = cur->next;
            idx++;
        }
    }
}

void remove_and_print_bgpid(list_t *bg_list, pid_t done){
    node_t *cur = bg_list->head;
    int idx = 0;

    while (cur != NULL) {
        bgentry_t *e = (bgentry_t *)cur->data;
        if (e != NULL && e->pid == done) {
            char *line_copy = strdup(e->job->line);

            RemoveByIndex(bg_list, idx);

            printf(BG_TERM, (int)done, line_copy);

            free(line_copy);
            return;
        }
        cur = cur->next;
        idx++;
    }
}

void sigchld_handler(int sig) {
    (void)sig;
    bg_child_terminated = 1;
}

void sigusr2_handler(int sig) {
    (void)sig;

    char buf[128];
    size_t i = 0;

    const char *prefix = "Num of Background processes: ";
    while (prefix[i] != '\0' && i < sizeof(buf) - 1) {
        buf[i] = prefix[i];
        i++;
    }

    int n = (int)bg_count;
    if (n < 0) n = 0;

    char digits[16];
    int d = 0;

    if (n == 0) {
        digits[d++] = '0';
    } else {
        while (n > 0 && d < (int)sizeof(digits)) {
            digits[d++] = (char)('0' + (n % 10));
            n /= 10;
        }
    }

    while (d > 0 && i < sizeof(buf) - 1) {
        buf[i++] = digits[--d];
    }

    if (i < sizeof(buf) - 1) buf[i++] = '\n';

    (void)write(STDERR_FILENO, buf, i);
}