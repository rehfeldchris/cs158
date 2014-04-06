/* tcpclient.c */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#define PORT 5000
#define MAX_BYTE 66000
#define SIZE_COUNT 7
#define SEND_COUNT 10
double time_diff(struct timeval x , struct timeval y)
{
   return (y.tv_sec - x.tv_sec) + 1e-6 * (y.tv_usec - x.tv_usec);
}
int main(int argc, char * argv[])

{
      int msg_sizes[SIZE_COUNT] = {1, 1024, 4*1024, 8*1024, 16*1024, 32*1024, 64*1024 };
        int sock, bytes_recieved,i,j,k;  
        char send_data[MAX_BYTE],recv_data[MAX_BYTE];
        char *hostname;
        struct hostent *host;
        struct sockaddr_in server_addr;  
        struct timeval start, stop;
        if (argc == 2) {
            hostname = argv[1];
         } else {
            printf("Using localhost\n");
            hostname = "127.0.0.1";
         }
         host = gethostbyname(hostname);
        if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            perror("Socket");
            exit(1);
        }

        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(PORT);   
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(server_addr.sin_zero),8); 
      for(k=0; k < SIZE_COUNT; k++) // Iterate over the different size of messages
      {
         printf("Size: %d\n", msg_sizes[k]);
         for(j=0; j< SEND_COUNT; j++)  // Send each message multiple times
         {
            for(i = 0; i < msg_sizes[k]; i++) // Create the actual message
            {
               send_data[i]='A';
            }
            send_data[msg_sizes[k]-1] = 'B'; // Null terminate the string
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // Create the socket
               perror("Socket");
            }
            if (connect(sock, (struct sockaddr *)&server_addr, // Connect
                    sizeof(struct sockaddr)) == -1) 
            {
               perror("Connect");
            } else {
               gettimeofday(&start, NULL);              
               send(sock,send_data,strlen(send_data)+1, 0);  
               bytes_recieved=recv(sock,recv_data,MAX_BYTE,0); // Get received count
          //     recv_data[bytes_recieved] = '\0';  // Null terminate the string
               gettimeofday(&stop, NULL);
               double elapsed = time_diff(start, stop);
               printf("#%d TOOK: %G TO GET BACK %d bytes\n", j+1, elapsed, bytes_recieved);

               close(sock); 
            } // Send
         } // SEND_COUNT
         sleep(2); // Sleep for easier viewing of output
      } // SIZE_COUNT
        
return 0;
}
