/* tcpserver.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_BUF 65537
#define PORT 5000
int main()
{
        int sock, connected, bytes_recieved , true = 1;  
        char send_data[MAX_BUF] , recv_data[MAX_BUF];       

        struct sockaddr_in server_addr,client_addr;    
        int sin_size;
        
        if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            perror("Socket");
            exit(1);
        }

        if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1) {
            perror("Setsockopt");
            exit(1);
        }
        
        server_addr.sin_family = AF_INET;         
        server_addr.sin_port = htons(PORT);     
        server_addr.sin_addr.s_addr = INADDR_ANY; 
        bzero(&(server_addr.sin_zero),8); 

        if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))
                                                                       == -1) {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(sock, 5) == -1) {
            perror("Listen");
            exit(1);
        }
    
  printf("\nTCPServer Waiting for client on port %d\n", PORT);


        while(1)
        {
		memset(&recv_data[0], 0, sizeof(recv_data));
		memset(&send_data[0], 0, sizeof(send_data));
		fflush(stdout);

            sin_size = sizeof(struct sockaddr_in);

            connected = accept(sock, (struct sockaddr *)&client_addr,&sin_size);

            printf("\nCONNECTION: (%s , %d)\n",
                   inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
			   
			   int count = 0;
			   count += recv(connected,recv_data,MAX_BUF,0);
			   printf("packet: %d bytes\n", count);
			   printf("Last char: %s\n",&recv_data[count-1]);
			   while(recv_data > 0 && recv_data[count-1] != 'B')
			   {
					int temp = recv(connected,&recv_data[count],MAX_BUF,0);
					count += temp;
					printf("packet: %d bytes\n", temp);
					printf("Last Char: %s\n",&recv_data[count-1]);
			   }
			   printf("End Of Message Found\n");
			   
			  
              printf("RECIEVED %d bytes\n" , count);
			  
			   int sent = send(connected,&recv_data[0],strlen(&recv_data[0]), 0);
			   printf("Sent: %d\n", sent);
               if( sent < 0)
			    perror("Sending");
			  
 			  printf("ClOSING: (%s , %d)\n\n",
                   inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
              close(connected);
        }       

      close(sock);
      return 0;
} 
