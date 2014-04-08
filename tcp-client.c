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
#define MAX_BYTE 65537
#define SIZE_COUNT 7
#define SEND_COUNT 100
double time_diff(struct timeval x , struct timeval y)
{
   return (y.tv_sec - x.tv_sec) + 1e-6 * (y.tv_usec - x.tv_usec);
}
int main(int argc, char * argv[])

{
      int msg_sizes[SIZE_COUNT] = {1, 1024, 4*1024, 8*1024, 16*1024, 32*1024, 64*1024 };
        int sock, bytes_recieved,i,j,k;  
        double totaltime = 0;
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
			memset(&recv_data[0], 0, sizeof(recv_data));
			memset(&send_data[0], 0, sizeof(send_data));
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
            } else 
			{
			fflush(stdout);

               gettimeofday(&start, NULL);
			   
			   int sent = send(sock,send_data,strlen(send_data), 0);
		//	   printf("#%d Sent: %d\n",j+1, sent);
               if( sent < 0)
			    perror("Sending");
		//		printf("#%d Sent: %d bytes\n", j+1, sent);
				
				int count = 0;
			   count += recv(sock,recv_data,MAX_BYTE,0);
	//		   printf("packet: %d bytes\n", count);
		//	   printf("Last char: %s\n",&recv_data[count-1]);
			   while(recv_data > 0 && recv_data[count-1] != 'B')
			   {
					int temp = recv(sock,&recv_data[count],MAX_BYTE,0);
					count += temp;
			//		printf("packet: %d bytes\n", temp);
		      //		printf("Last Char: %s\n",&recv_data[count-1]);
			   }
	//		   printf("End Of Message Found\n");

			   
			  
     //         printf("Recieved %d bytes\n" , count);
			  
			  
               gettimeofday(&stop, NULL);
               double elapsed = time_diff(start, stop);
               totaltime += elapsed;
     //          printf("#%d TOOK %G seconds to SEND %d bytes and RECEIVE %d\n bytes\n" , j+1, elapsed, sent, count);
	//		   printf("#%d ClOSING\n\n", j+1);
               close(sock);
            } // Send
         } // SEND_COUNT
         printf("Average RTT: %G\n", totaltime/SEND_COUNT);
         printf("Average Throughput: %G\n", msg_sizes[k]/totaltime);
		 printf("Total time: %G\n", totaltime);
      } // SIZE_COUNT
        
return 0;
}
