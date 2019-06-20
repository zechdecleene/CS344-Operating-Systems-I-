#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>


void catchSIGINTMAIN(int);
void catchSIGTSTP(int);
bool startsWith(const char*, const char*);
bool endsWith(const char*, const char*);
bool dontIgnore;

int main(){
  //SIGINT catcher
  struct sigaction SIGINT_action = {0};
  SIGINT_action.sa_handler = SIG_IGN;
  sigaction(SIGINT, &SIGINT_action, NULL);
  //SIGTSTP catcher
  struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = catchSIGTSTP;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = 0;
  sigaction(SIGTSTP, &SIGTSTP_action, 0);

  dontIgnore = true;

  int exitcode = 0;
  char input[2048];
  char tmpString[2048];
  char *in = NULL;
  size_t bufferSize = 2047;
  int numchars;
  char args[512][64]; //ARGS to be passed into exec
  char allArgs[512][64];
  int j;
  for(j = 0; j < 512; j++){
    memset( args[j], '\0', sizeof(char)*64);
  }

  int saved_stdout = dup(1);
  int saved_stdin = dup(0);

  pid_t spawnPID = -5;
  int childExitStatus = -5;
  int bgpid[64];
  int numbg = 0;

  int i = 0;
  int numargs = 0;
  bool bg = false;
  bool useTmp = false;
  //Infinite Loop
  while(1){



    numchars = 0;
    i = 0;
    bg = false;
    useTmp = false;
    do{
      printf(": ");
      fflush(stdout);
      numchars = getline(&in, &bufferSize, stdin);
      if(numchars == -1){
        clearerr(stdin);
      }else{
        int k;
        for(k = 0; k < numchars; k++){
          if(in[k] == '$'){
            if(in[k+1] != '\0'){
              if(in[k+1] == '$'){
                in[k] = '%';
                in[k+1] = 'i';
                int tmpPID = getpid();
                sprintf(tmpString, in, tmpPID);
                printf("%s\n", tmpString);
                useTmp = true;
                break;
              }
            }
          }
        }


      }


    }while(numchars == -1);
    if(useTmp){
      strcpy(input, tmpString);
    }else{
      strcpy(input, in);
    }
    input[strcspn(input, "\n")] = 0;
    //Tokenize the string
    char str[2048];
    strcpy(str, input);
    char *token;
    char *r = str;
    while((token = strtok_r(r, " ><!&|", &r))){
      if(token == 0){
        printf("WE Got One Boss\n");
      }else if(token != NULL){
        strcpy(args[i], token);
        // args[i][strcspn(args[i], "\n")] = 0;
        i++;
      }
    }
    numargs = i;

    // for(i=0; i<numargs;i++){
    //   printf("%s\n", args[i]);
    // }

    i = 0; //Reset I for 2nd time
    strcpy(str, input);
    char *token2;
    char *r2 = str;
    while((token2 = strtok_r(r2, " ", &r2))){
      if(token2 == 0){
        printf("WE Got One Boss\n");
      }else if(token2 != NULL){
        strcpy(allArgs[i], token2);
        // args[i][strcspn(args[i], "\n")] = 0;
        i++;
      }
    }

    int numAllArgs = i;
    int result;
    bool targetOpen = false, sourceOpen = false;
    int TargetFD, SourceFD;
    for(j = 0; j < numAllArgs; j++){
      if(!strcmp(allArgs[j], ">")){
        //Redirect stdout to file
        TargetFD = open(allArgs[j+1], O_CREAT | O_WRONLY | O_TRUNC, 0777);
        result = dup2(TargetFD, 1);
        targetOpen = true;
        args[j][0] = '\0';
        numargs--;
        // fcntl(TargetFD, F_SETFD, FD_CLOEXEC);
        // if(args[j+1][0] != '\0'){
        //   strcpy(args[j], args[j+1]);
        //   args[j+1][0] = '\0';
        //   numargs--;
        // }
      }
      if(!strcmp(allArgs[j], "<")){
        //Redirect stdin to file
        SourceFD = open(allArgs[j+1], O_RDONLY);
        if(errno == ENOENT){
          printf("BAD FILENAME, Does not exist\n");
          fflush(stdout);
        }else{
        result = dup2(SourceFD, 0);
        sourceOpen = true;
        args[j][0] = '\0';
        numargs--;
        }
        // fcntl(SourceFD, F_SETFD, FD_CLOEXEC);
        // if(args[j+1][0] != '\0'){
        //   strcpy(args[j], args[j+1]);
        //   args[j+1][0] = '\0';
        //   numargs--;
        // }
      }
    }

    //Periodic check if bg process are done
    int idx;
    for(idx = 0; idx < numbg; idx++){
        if(waitpid(bgpid[idx], &childExitStatus, WNOHANG) > 0){

          if(WIFSIGNALED(childExitStatus) != 0){
            exitcode = WTERMSIG(childExitStatus);
            printf("Background process PID:%d\nTerminated with error code: %d\n", bgpid[idx], exitcode);
            fflush(stdout);
          }
          if(WIFEXITED(childExitStatus) != 0){
            exitcode = WEXITSTATUS(childExitStatus);
            printf("Background process PID:%d\nTerminated with error code: %d\n", bgpid[idx], exitcode);
            fflush(stdout);
          }
        }
    }

    if(!strcmp(allArgs[numAllArgs-1], "&") && dontIgnore){
      bg = true;
    }

    if(input[0] == '#'){
      printf("%s\n", input);
      fflush(stdout);
    }else
    if(!strcmp(input, "exit")){
      exit(1);
    }else
    if(startsWith("cd", input)){
      if(numargs == 1){
        char *homeDir = getenv("HOME");
        chdir(homeDir);
      }else{
        char currDir[1024];
        char newDir[1024];
        getcwd(currDir, sizeof(currDir));
        strcpy(newDir, currDir);
        strcat(newDir, "/");
        strcat(newDir, args[1]);
        chdir(newDir);
        perror("");
      }
    }else
    if(startsWith(input, "status")){
      printf("Exit Value: %d\n", exitcode);
      fflush(stdout);
    }else if(!strcmp(input, "")){

    }else{
      //This is where all the non-built in commands run
      char *execArgs[512];
      for(i = 0; i < numargs; i++){
        execArgs[i] = args[i];
      }





      execArgs[numargs] = NULL;
      spawnPID = fork();
      switch(spawnPID){
        case -1: {perror("HULL Breach!\n"); break; }
        case 0: {
          if(!bg){
            SIGINT_action.sa_handler = SIG_DFL;
            sigaction(SIGINT, &SIGINT_action, NULL);
          }
          exitcode = execvp(args[0], execArgs);
          perror("Bad command!\n");
          printf("Exit Code: %d\n", exitcode);
          fflush(stdout);
          exit(0);
          break;
        }
        default: {
          if(bg){
            bgpid[numbg] = spawnPID;
            numbg++;
            printf("Background process ID: %d\n", spawnPID);
            fflush(stdout);
          }
          if(!bg){
            waitpid(spawnPID, &childExitStatus, NULL);
          }

          if(WIFSIGNALED(childExitStatus) != 0){
            exitcode = WTERMSIG(childExitStatus);
          }
          if(WIFEXITED(childExitStatus) != 0){
            exitcode = WEXITSTATUS(childExitStatus);
          }
          if(targetOpen){
            dup2(saved_stdout, 1);
            // close(saved_stdout);
            targetOpen = false;
          }
          if(sourceOpen){
            dup2(saved_stdin, 0);
            // close(saved_stdin);
            sourceOpen = false;
          }
          break;
        }
      }


    }


  }



}


void catchSIGINTMAIN(int signo){
  printf("\n");
  fflush(stdout);
  exit(signo);
}

void catchSIGTSTP(int signo){
  if(dontIgnore){
    char *messageEnter = "\nEntering foreground-only mode.\n"; //32
    write(STDOUT_FILENO, messageEnter, 32);
    dontIgnore = false;
  }else{
    char *messageExit = "\nExiting foreground-only mode.\n"; //31
    write(STDOUT_FILENO, messageExit, 31);
    dontIgnore = true;
  }
  return;
}

bool startsWith(const char *pre, const char *str){
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

bool endsWith (const char* str, const char* tail){
  int string_len = strlen(str);
  int tail_len = strlen(tail);
  int test = strcmp(str + string_len - tail_len, tail);
  if(!strcmp(str + string_len - tail_len, tail)){
    return 1;
  }
  return 0;
}
