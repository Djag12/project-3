#include "server.h"

int chat_serv_sock_fd; //server socket
struct room *rooms_head = NULL; // Global variable for room management
/////////////////////////////////////////////
// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE

int numReaders = 0; // keep count of the number of readers

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex lock
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  // read/write lock

/////////////////////////////////////////////


char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

struct node *head = NULL;


int main(int argc, char **argv) {

   signal(SIGINT, sigintHandler);
   // Initialize the default room
   pthread_mutex_lock(&rw_lock);
   head = insertFirstU(head, -1, DEFAULT_ROOM); // Default room
   pthread_mutex_unlock(&rw_lock);
   printf("Default room '%s' created.\n", DEFAULT_ROOM);
    
   //////////////////////////////////////////////////////
   // create the default room for all clients to join when 
   // initially connecting
   //  
   //    TODO
   //////////////////////////////////////////////////////

   // Open server socket
   chat_serv_sock_fd = get_server_socket("localhost", "8888");

   // step 3: get ready to accept connections
   if(start_server(chat_serv_sock_fd, BACKLOG) == -1) {
      printf("start server error\n");
      exit(1);
   }
   
   printf("Server Launched! Listening on PORT: %d\n", PORT);
    
   //Main execution loop
   while(1) {
      //Accept a connection, start a thread
      int new_client = accept_client(chat_serv_sock_fd);
      if(new_client != -1) {
         pthread_t new_client_thread;
         pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
      }
   }

   close(chat_serv_sock_fd);
}


int get_server_socket(char *hostname, char *port) {
    int opt = TRUE;   
    int master_socket;
    struct sockaddr_in address; 
    
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   

   return master_socket;
}


int start_server(int serv_socket, int backlog) {
   int status = 0;
   if ((status = listen(serv_socket, backlog)) == -1) {
      printf("socket listen error\n");
   }
   return status;
}


int accept_client(int serv_sock) {
   int reply_sock_fd = -1;
   socklen_t sin_size = sizeof(struct sockaddr_storage);
   struct sockaddr_storage client_addr;
   //char client_printable_addr[INET6_ADDRSTRLEN];

   // accept a connection request from a client
   // the returned file descriptor from accept will be used
   // to communicate with this client.
   if ((reply_sock_fd = accept(serv_sock,(struct sockaddr *)&client_addr, &sin_size)) == -1) {
      printf("socket accept error\n");
   }
   return reply_sock_fd;
}


/* Handle SIGINT (CTRL+C) */
void sigintHandler(int sig_num) {
   printf("Error:Forced Exit.\n");

   //////////////////////////////////////////////////////////
   //Closing client sockets and freeing memory from user list
   printf("\nServer is shutting down...\n");
   pthread_mutex_lock(&rw_lock);
   struct node *current = head;
   char buffer[MAXBUFF];
   sprintf(buffer, "Server shutting down in 5 seconds. Please disconnect.\n");
   while (current != NULL) {
       if (current->socket > 0) {
           send(current->socket, buffer, strlen(buffer), 0);
           close(current->socket);
       }
       struct node *temp = current;
       current = current->next;
       free(temp);
   }
   pthread_mutex_unlock(&rw_lock);
   sleep(5);
   //
   //       TODO
   //////////////////////////////////////////////////////////

   printf("--------CLOSING ACTIVE USERS--------\n");

   close(chat_serv_sock_fd);
   exit(0);
}