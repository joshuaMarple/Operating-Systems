#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#define BSIZE 4096

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
  int pipefd_1[2];
  int pipefd_2[2];
  int pipefd_3[2];
  // char buf;
  if (pipe(pipefd_1) == -1){
    perror("pipe");
    exit(1);
  }
  if (pipe(pipefd_2) == -1){
    perror("pipe");
    exit(1);
  }
  if (pipe(pipefd_3) == -1){
    perror("pipe");
    exit(1);
  }
  int status;

  pid_t pid_1, pid_2, pid_3, pid_4;

  if (argc != 4) {
    printf("usage: finder DIR STR NUM_FILES\n");
    exit(0);
  }

  pid_1 = fork();
  if (pid_1 == 0) {
    char cmdbuf[BSIZE];
    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]);
    dup2(pipefd_1[1], STDOUT_FILENO);
    
    close(pipefd_1[0]);
    close(pipefd_1[1]);
    close(pipefd_2[0]);
    close(pipefd_2[1]);
    close(pipefd_3[0]);
    close(pipefd_3[1]);
    
    if ( (execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0) {
      fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
      return EXIT_FAILURE;
    }

  }

  pid_2 = fork();
  if (pid_2 == 0) {
    char cmdbuf[BSIZE];
    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s %s -c %s", XARGS_EXEC, GREP_EXEC, argv[2]);
    dup2(pipefd_1[0], STDIN_FILENO);
    dup2(pipefd_2[1], STDOUT_FILENO);
    
    close(pipefd_1[0]);
    close(pipefd_1[1]);
    close(pipefd_2[0]);
    close(pipefd_2[1]);
    close(pipefd_3[0]);
    close(pipefd_3[1]);

    if ( (execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0) {
      fprintf(stderr, "\nError execing grep. ERROR#%d\n", errno);
      return EXIT_FAILURE;
    }
  }

  pid_3 = fork();
  if (pid_3 == 0) {
    char cmdbuf[BSIZE];
    bzero(cmdbuf, BSIZE);
    dup2(pipefd_2[0], STDIN_FILENO);
    dup2(pipefd_3[1], STDOUT_FILENO);
    sprintf(cmdbuf, "%s -t : +1.0 -2.0 --numeric --reverse", SORT_EXEC);
    
    close(pipefd_1[0]);
    close(pipefd_1[1]);
    close(pipefd_2[0]);
    close(pipefd_2[1]);
    close(pipefd_3[0]);
    close(pipefd_3[1]);

    if ( (execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0) {
      fprintf(stderr, "\nError execing sort. ERROR#%d\n", errno);
      return EXIT_FAILURE;
    }
  }

  pid_4 = fork();
  if (pid_4 == 0) {
    char cmdbuf[BSIZE];
    bzero(cmdbuf, BSIZE);
    dup2(pipefd_3[0], STDIN_FILENO);
    sprintf(cmdbuf, "--lines=%s", argv[3]);
    
    close(pipefd_1[0]);
    close(pipefd_1[1]);
    close(pipefd_2[0]);
    close(pipefd_2[1]);
    close(pipefd_3[0]);
    close(pipefd_3[1]);

    if ( (execl(HEAD_EXEC, cmdbuf, (char *) 0)) < 0) {
      fprintf(stderr, "\nError execing sort. ERROR#%d\n", errno);
      return EXIT_FAILURE;
    }
  }

  close(pipefd_1[0]);
  close(pipefd_1[1]);
  close(pipefd_2[0]);
  close(pipefd_2[1]);
  close(pipefd_3[0]);
  close(pipefd_3[1]);

  if ((waitpid(pid_1, &status, 0)) == -1) {
    fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_2, &status, 0)) == -1) {
    fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_3, &status, 0)) == -1) {
    fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_4, &status, 0)) == -1) {
    fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  return 0;
}