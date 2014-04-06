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

#define MAX_BUF 128000
#define PORT 5000

int checkMessage()
{

}
int main()
{
        int sock, connected, bytes_recieved, bytes_sent, true = 1;  
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
        fflush(stdout);


        while(1)
        {  
            int total = 0;
            int i;
            int done = 0;
            sin_size = sizeof(struct sockaddr_in);

            connected = accept(sock, (struct sockaddr *)&client_addr,&sin_size);

            printf("\n I got a connection from (%s , %d)",
                   inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            while(done == 0)
            {
              bytes_recieved = recv(connected,recv_data,MAX_BUF+total,total);
              total+=bytes_recieved;
              for(i = 0; i < total; i++)
              {
                if(recv_data[i] == 'B')
                {
                  done = 1;
                  printf("\nRECIEVED %d bytes" , total);
                  printf("\nNow Sending: %d after receiving %d", strlen(recv_data), bytes_recieved);
                  if(bytes_sent = send(connected, recv_data,total, 0) < total)
                  {
                    printf("Did not complete sending");
                  }  
                  break;
                }
              }
            }              
            close(connected);
        }       

      close(sock);
      return 0;
} 
