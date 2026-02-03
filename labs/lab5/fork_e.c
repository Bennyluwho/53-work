#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  pid_t pid;
  int x;

  FILE *fd = fopen("nums.txt", "r");
  if (fd == NULL) {
    printf("Error: Could not open file");
    exit(0);
  }

  pid = fork();
  if (pid == 0) {
    /*this is the child*/
    while (fscanf(fd, "%d", &x) == 1) {
      printf("child: %d\n", x);
    }
    exit(0);
  }

  /* parent code */
  while (fscanf(fd, "%d", &x) == 1) {
    printf("parent: %d\n", x);
  }

  exit(0);
}
