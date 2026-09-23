#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
fork et exec, on suggère d’utiliser la version execvp,
• pipe pour créer un tube,
• dup2(fd1,fd2) pour remplacer fd2 par fd1, de façon à substituer aux entrées sorties “normales”
les extrémités du pipe
On pourra tester avec par exemple (le backslash protège l’interprétation du pipe par le shell) :
pipe /bin/cat pipe.cpp \| /bin/wc -l

*/

int main(int argc, char **argv) {

  // Parse command line arguments: find the pipe separator "|"
  // Format: ./pipe cmd1 [args...] | cmd2 [args...]
  //./pipe /bin/cat ../src/pipe.cpp \| /bin/wc -l
  //74

  // Create a pipe for inter-process communication
  int pipefd[2];
  int status;
  pipe(pipefd);
  pid_t pid_pere, pid_enfant1,pid_enfant2;
  pid_pere = getpid();
  //message c'est dans argv
  auto cmd1 = &argv[1]; //le premier argument est le nom du programme
  char **cmd2 = nullptr;
  for(int i=1; i<argc; i++){
    if(strcmp(argv[i],"|") == 0){
      argv[i] = nullptr; //on remplace le | par un nullptr pour execvp
      cmd2 = &argv[i+1]; //le premier argument le nom du programme
      break;
    }
  }
  // Fork the first child process ; l'enfant redirige la sortie vers l'extrémité d'écriture du tube, puis exécute
  pid_enfant1 = fork();
  if( pid_enfant1 == 0){
    dup2(pipefd[1], STDOUT_FILENO);  //stdout -> pipe
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(cmd1[0], cmd1);
    exit(1);
  }
  
  // Fork the second child process ; child redirects in from read end of pipe, then exec
  pid_enfant2 = fork();
  if( pid_enfant2 == 0){
    dup2(pipefd[0], STDIN_FILENO);  //stdin -> pipe
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(cmd2[0], cmd2);
    exit(1);
  }

  // Wait for both children to finish
  close(pipefd[0]);
  close(pipefd[1]);

  waitpid(pid_enfant1,&status, 0);
  waitpid(pid_enfant2,&status, 0);
  
  return 0;
}
