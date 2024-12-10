#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
   char username[30];
   int socket;
   struct node *next;
};



/////////////////// USERLIST //////////////////////////

//insert node at the first location
struct node* insertFirstU(struct node *head, int socket, char *username);

//find a node with given username
struct node* findU(struct node *head, char* username);

struct room {
   char name[30];
   struct node *members; // Linked list of users in the room
   struct room *next;
};

// Functions for room management
struct room* createRoom(struct room *head, char *room_name);
struct room* findRoom(struct room *head, char* room_name);

// Extern declarations
extern struct room *rooms_head;
