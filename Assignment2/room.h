#ifndef ROOM
#define ROOM

struct Room{
  char *name;
  char *connections[6];
  int num_connections;
  char *type;
};

#endif
