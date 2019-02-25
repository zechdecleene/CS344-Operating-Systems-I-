#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

int main(int argc, char** argv){
  if(argc < 2){
    printf("Error, not enough arguments. USAGE\n");
    exit(0);
  }
  time_t t;
  srand((unsigned) time(&t));
  char* arg= argv[1];
  int i;
  for(i = 0; i < strlen(arg); i++){
    if(!isdigit(arg[i])){
      printf("ERROR ARG NOT INT.\n" );
      exit(0);
    }
  }
  int keySize = atoi(argv[1]);
  char* key;
  int rndchar;
  key = (char *) malloc((size_t)keySize+1);
  memset(key, '\0', sizeof(key));
  for(i = 0; i < keySize; i++){
    key[i] = (rand() % 27) + 65;
    if(key[i] == 91){
      key[i] = 32;
    }
  }
  printf("%s\n", key);
  free(key);
}
