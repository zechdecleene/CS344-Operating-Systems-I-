#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "room.h"
#include "args.h"

#define NUM_THREADS 7
#define NUM_ROOMS 7

void* do_work();
bool IsGraphFull(struct Room *room);
struct Room GetRandomRoom(struct Room *room);
void AddRandomConnection(struct Room*);
void printRooms(struct Room*);
bool ConnectionExists(struct Room*, int r1, int r2);
bool NameBeenUsed(int, int*, int);

int main(){
  // Hard-Coded room names
  char *room_name[10];
  room_name[0] = "RoomA";
  room_name[1] = "RoomB";
  room_name[2] = "RoomC";
  room_name[3] = "RoomD";
  room_name[4] = "RoomE";
  room_name[5] = "RoomF";
  room_name[6] = "RoomG";
  room_name[7] = "RoomH";
  room_name[8] = "RoomI";
  room_name[9] = "RoomJ";
  int namesUsed[7];
  int result_code, index;
  pthread_t threads[7];
  time_t t;

  //Initialize Random Number Generator
  srand((unsigned) time(&t));
  struct Room rooms[7];

  //Initalize array of Rooms
  for(index = 0; index < NUM_ROOMS; index++){
    int nameindex;
    nameindex = rand() % 10;
    while(NameBeenUsed(nameindex, namesUsed, index) == true){
      nameindex = rand() % 10;
    }
    namesUsed[index] = nameindex;
    rooms[index].name =room_name[nameindex];
    rooms[index].num_connections = 0;
    if(index == 0){
      // printf("Room %d is the START_ROOM\n", index);
      rooms[index].type = "START_ROOM";
    }
    else if(index == 6){
      // printf("Room %d is the END_ROOM\n", index);
      rooms[index].type = "END_ROOM";
    }
    else{
      //printf("Room %d is a MID_ROOM\n", index);
      rooms[index].type = "MID_ROOM";
    }
  }

  //Create all Connections
  while(IsGraphFull(rooms) == false){
    AddRandomConnection(rooms);
  }

  //printRooms(rooms);

  //Create args to send to threads
  struct Args args[7];
  for(index = 0; index < NUM_ROOMS; index++){
    args[index].room = rooms;
    args[index].idx = index;
    //Thread Initialization
    //printf("Creating Thread: %d\n" , args[index].idx);
    result_code = pthread_create(&threads[index], NULL, do_work, (void *) &args[index]);
    assert(result_code == 0);
  }


  for(index = 0; index < NUM_THREADS; index++){
    result_code = pthread_join(threads[index], NULL);
    //printf("Completed Thread: %d\n", index);
    assert(result_code == 0);
  }

  //printf("ALL Threads Successful.\n");

}

bool NameBeenUsed(int num, int *usedNums, int idx){
  int i;
  for(i=0; i < idx; i++){
    if(usedNums[i] == num){
      return true;
    }
  }
  return false;
}

void AddRandomConnection(struct Room *rooms){
  int random1, random2;
  do{
    random1 = rand() % 7;
  }while(!(rooms[random1].num_connections < 6));

  do{
    random2 = rand() % 7;
  }while(ConnectionExists(rooms, random1, random2) || rooms[random2].num_connections >= 6 || random2 == random1);

  //Connect the rooms
  int i = rooms[random1].num_connections;
  rooms[random1].connections[i] = rooms[random2].name;
  rooms[random1].num_connections++;

  int j = rooms[random2].num_connections;
  rooms[random2].connections[j] = rooms[random1].name;
  rooms[random2].num_connections++;
}

bool ConnectionExists(struct Room *rooms, int r1, int r2){
  int i;
  for(i = 0; i < rooms[r1].num_connections; i++){
    if(rooms[r1].connections[i] == rooms[r2].name){
      // printf("Match!\n");
      return true;
    }
  }

  return false;
}

void printRooms(struct Room *rooms){
  int i,j;
  for(i = 0; i < NUM_ROOMS; i++){
    printf("ROOM NAME: %s\n", rooms[i].name);
    for(j = 0; j < rooms[i].num_connections; j++){
      printf("CONNECTION %d: %s\n", j, rooms[i].connections[j]);
    }
    printf("ROOM TYPE: %s\n", rooms[i].type);
    printf("\n");
  }
}

bool IsGraphFull(struct Room *rooms){
  int i;
  for(i = 0; i < NUM_ROOMS; i++){
    if(rooms[i].num_connections < 3){
      return false;
    }
  }

  return true;
}

struct Room GetRandomRoom(struct Room *rooms){
  int randomRoom = rand() % 7;
  return rooms[randomRoom];
}

void* do_work(void *argument){
  //Create file of struct
  struct Args args;
  args = *((struct Args*) argument);
  //printf("Inside Thread: %d\n" , args.idx);

  //Write to file
  FILE *fp;
  int i;
  int num = pthread_self();

  //Dynamic file naming
  char buffer[32]; //File name
  snprintf(buffer, sizeof(char) * 32, "decleenz.rooms.%d", num);
  fp = fopen(buffer, "wb");

  fprintf(fp, "ROOM NAME: %s\n", args.room[args.idx].name);
  for(i = 0; i < args.room[args.idx].num_connections; i++){
    fprintf(fp, "CONNECTION %d: %s\n", (i+1), args.room[args.idx].connections[i]);
  }
  fprintf(fp, "ROOM TYPE: %s\n", args.room[args.idx].type);

  fclose(fp);
}
