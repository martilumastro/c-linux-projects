#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>

static void print_ids(const char *label){
  uid_t real_uid = getuid();
  uid_t effective_uid = geteuid();

  printf("[%s] Real UID: %u\n", label, (unsigned int)real_uid);
  printf("[%s] Effective UID: %u\n", label, (unsigned int)effective_uid);
}


static int run_helper(void){
  pid_t pid;
  pid = fork();
  if(pid < 0){
    perror("fork() failed");
    return -1;
  }
  if(pid == 0){
    execl("./suid_helper", "suid_helper", (char *)NULL);
    perror("execl() failed");
    _exit(EXIT_FAILURE);
  }

  int status;
  if(waitpid(pid, &status, 0) < 0){
    perror("waitpid() failed");
    return -1;
  }
  if(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS){
    printf("The child process terminated successfully\n");
  } else {
    fprintf(stderr, "The child process failed\n");
    return -1;
  }
  return 0;
}


static int enable_no_new_privs(void){

  if(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0){
    perror("prctl() failed");
    return -1;
  }
  return 0;
}


int main(void){
    printf("Execution without NoNewPrivs\n");
    print_ids("Parent before NNP");
    if(run_helper() < 0)
        exit(EXIT_FAILURE);
    printf("Result of first execution: the child process should have been able to execute successfully.\n");

    printf("\nExecution with NoNewPrivs\n");
    if(enable_no_new_privs() < 0)
        exit(EXIT_FAILURE);
    printf("Successfully enabled NoNewPrivs\n");
    print_ids("Parent after NNP");
    if(run_helper() < 0)
        exit(EXIT_FAILURE);
    printf("Result of second execution: SUID privilege escalation was prevented.\n");

    return EXIT_SUCCESS;
}