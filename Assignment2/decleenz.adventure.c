#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <dirent.h>
#include <fcntl.h>
#include "room2.h"


#define NUM_ROOMS 7

bool startsWith(const char *, const char *);
bool NotAtEnd(int, struct RoomTypeB *, int);
void* tellTime(void*);


int main(){
  const char *filename = "decleenz.rooms.";
  struct dirent *de;  // Pointer for directory entry



  // opendir() returns a pointer of DIR type.
  DIR *dr = opendir(".");

  if (dr == NULL)  // opendir returns NULL if couldn't open directory
  {
      printf("Could not open current directory" );
      return 0;
  }
  FILE *file;
  struct stat fileStat;
  //Find most recent rooms
  int highest = 0;
  while ((de = readdir(dr)) != NULL){
          if(startsWith(filename, de->d_name)){
            //Find most recent -"filename.xxxxxx" modified time
            stat(de->d_name, &fileStat);
            if(fileStat.st_mtime > highest){
              highest = fileStat.st_mtime;
            }
          }
  }

  char buffer[32];
  int file_descriptor;
  int thisFile, i, j;

  struct RoomTypeB rooms[10];

  DIR *dr2 = opendir(".");
  //Now that I know the most recent, go through files again and load in data
  i = 0;
  j = 0;
  while((de = readdir(dr2)) != NULL){
    if(startsWith(filename, de->d_name)){
      stat(de->d_name, &fileStat);
      thisFile = fileStat.st_mtime;
      if(thisFile == highest){
        file_descriptor = open(de->d_name, O_RDONLY);
        lseek(file_descriptor, 11, SEEK_SET); //Sets file pointer to beginning of Room Name
        read(file_descriptor, &buffer, 5);
        strcpy(rooms[i].name, buffer);
        // printf("Room: %s\n", rooms[i].name);
        memset(buffer, 0, strlen(buffer));
        //Check if next line starts with 'C'
        while(true){
          lseek(file_descriptor, 1, SEEK_CUR);
          read(file_descriptor, &buffer, 1);
          if(buffer[0] != 'C'){
            break;
          }
          lseek(file_descriptor, 13, SEEK_CUR);
          read(file_descriptor, &buffer, 5);
          strcpy(rooms[i].connections[j] , buffer);
          // printf("Connection: %s\n", rooms[i].connections[j]);
          memset(buffer, 0, strlen(buffer));
          j++;
        }
        rooms[i].num_connections = j;

        lseek(file_descriptor, 10, SEEK_CUR);
        read(file_descriptor, &buffer, 11);
        strcpy(rooms[i].type , buffer);
        // printf("Type: %s\n", rooms[i].type);
        memset(buffer, 0, strlen(buffer));
        j = 0;
        i++;
      }
    }
  }

  //At this point Struct Room rooms[7] is a fully decked out struct. Now for the actual game.
  //Get starting point.
  int index;
  int currentIdx;
  int result_code;


  int numCharsEntered = -5;
  int currChar = -5;
  size_t bufferSize = 0;
  char *lineEntered = NULL;

  //Get rid of pesky trailing newlines
  for(index = 0; index < NUM_ROOMS; index++){
    rooms[index].name[ strcspn(rooms[index].name , "\r\n") ] = '\0';
    rooms[index].type[ strcspn(rooms[index].type , "\r\n") ] = '\0';

    for(i = 0; i < rooms[index].num_connections; i++){
      rooms[index].connections[i][ strcspn(rooms[index].connections[i] , "\r\n") ] = '\0';
    }
  }

  for(index = 0; index < NUM_ROOMS; index++){
    if(!strcmp(rooms[index].type, "START_ROOM")){
      currentIdx = index;
      printf("CURRENT LOCATION: %s\n" ,rooms[index].name);
      printf("POSSIBLE CONNECTIONS: ");

      for(j = 0; j < rooms[index].num_connections; j++){
        printf("%s ", rooms[index].connections[j]);
      }
      printf("\n");

    }
  }
  bool roomexists = false;
  int stepCounter = 0;
  char roomsVisited[32][32];
  do{
  while(1){
    printf("Where to? > ");
    numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
    if(numCharsEntered != -5){
      break;
    }
  }

  lineEntered[ strcspn(lineEntered , "\r\n") ] = '\0';
  if(!strcmp(lineEntered, "time")){
    pthread_t thread;
    pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
    result_code = pthread_create(&thread, NULL, tellTime, &myMutex);
    result_code = pthread_join(thread, NULL);
    pthread_mutex_lock(&myMutex);
    roomexists = true;
    pthread_mutex_unlock(&myMutex);
  }


  for(i = 0; i < NUM_ROOMS; i++){
    if(!strcmp(lineEntered, rooms[i].name)){
      roomexists = true;
      currentIdx = i;
      strcpy(roomsVisited[stepCounter] , rooms[i].name);
      stepCounter++;
      break;
    }
  }
}while(!roomexists);

while(NotAtEnd(currentIdx, rooms, stepCounter)){
  roomexists = false;
  printf("CURRENT LOCATION: %s\n", rooms[currentIdx].name );
  printf("POSSIBLE CONNECTIONS: ");

  for(j = 0; j < rooms[currentIdx].num_connections; j++){
    printf("%s ", rooms[currentIdx].connections[j]);
  }
  printf("\n");
  while(1){
    printf("Where to? > ");
    numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
    if(numCharsEntered != -5){
      break;
    }
  }

  lineEntered[ strcspn(lineEntered , "\r\n") ] = '\0';
  if(!strcmp(lineEntered, "time")){
    pthread_t thread;
    pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
    result_code = pthread_create(&thread, NULL, tellTime, &myMutex);
    result_code = pthread_join(thread, NULL);
    pthread_mutex_lock(&myMutex);
    roomexists = true;
    pthread_mutex_unlock(&myMutex);
  }

  for(i = 0; i < NUM_ROOMS; i++){
    if(!strcmp(lineEntered, rooms[i].name)){
      roomexists = true;
      currentIdx = i;
      strcpy(roomsVisited[stepCounter] , rooms[i].name);
      stepCounter++;
      break;
    }
  }
  if(!roomexists){
    printf("Huh? I didn't get that.\n");
  }
}
  printf("Your path to victory was...\n");
  int k;
  for(k = 0; k < stepCounter; k++){
    printf("%s\n", roomsVisited[k]);
  }

  closedir(dr);
  return 0;
}

void* tellTime(void * argument){
  pthread_mutex_t *newMutex;
  newMutex = ((pthread_mutex_t *) argument);
  pthread_mutex_lock(newMutex);

  //create inner thread


  time_t currentTime;

  struct tm *timeinfo;
  char buffer[256];

  time(&currentTime);
  timeinfo = localtime(&currentTime);

  strftime(buffer, 256, "%I:%M %p, %A, %B %d, %Y", timeinfo);
  printf("\n%s\n\n", buffer);

  pthread_mutex_unlock(newMutex);

  FILE *file;
  file = fopen("currentTime.txt", "w");
  fprintf(file, "%s\n", buffer);
}

bool NotAtEnd(int CurrentIdx, struct RoomTypeB *rooms, int stepCounter){
  if(!strcmp(rooms[CurrentIdx].type, "END_ROOM")){
    printf("You Found the End Room! Congradulations!\n");
    printf("You Took %d Steps to find the end!\n", stepCounter);


    return false;
  }
  return true;
}

//Function checks to see if a string (*str) starts with another string (*pre)
bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
