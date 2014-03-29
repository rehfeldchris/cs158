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
#define MAX_BYTE 64001
#define SIZE_COUNT 7
double time_diff(struct timeval x , struct timeval y)
{
   return (y.tv_sec - x.tv_sec) + 1e-6 * (y.tv_usec - x.tv_usec);
}
int main()

{
      int msg_sizes[SIZE_COUNT] = {1, 1024, 4*1024, 8*1024, 16*1024, 32*1024, 64000 };
        int sock, bytes_recieved,i,j,k;  
        char send_data[MAX_BYTE],recv_data[MAX_BYTE];
        struct hostent *host;
        struct sockaddr_in server_addr;  
        struct timeval start, stop;
        host = gethostbyname("127.0.0.1");

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }

        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(PORT);   
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(server_addr.sin_zero),8); 
      for(k=0; k < SIZE_COUNT; k++)
      {
         printf("Size: %d\n", msg_sizes[k]);
         for(j=0; j< 10; j++)
         {
            for(i = 0; i < msg_sizes[k]; i++)
            {
               send_data[i]='A';
            }
            send_data[msg_sizes[k]] = '\0';
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
               perror("Socket");
            }
            if (connect(sock, (struct sockaddr *)&server_addr,
                    sizeof(struct sockaddr)) == -1) 
            {
               perror("Connect");
            } else {
               gettimeofday(&start, NULL);
               send(sock,send_data,strlen(send_data), 0);  
               bytes_recieved=recv(sock,recv_data,MAX_BYTE,0);
               recv_data[bytes_recieved] = '\0'; 
               gettimeofday(&stop, NULL);
               double elapsed = time_diff(start, stop);
               printf("#%d TOOK: %G\n", j, elapsed);
               close(sock); 
            }
         }
      }
        
return 0;
}
