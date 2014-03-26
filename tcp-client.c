
/* Sample TCP client */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
double time_diff(struct timeval x , struct timeval y)
{
   return (y.tv_sec - x.tv_sec) + 1e-6 * (y.tv_usec - x.tv_usec);
}
int main(int argc, char**argv)
{
   int sockfd,n,i;
   struct sockaddr_in servaddr,cliaddr;
   char sendline[1000];
   char recvline[1000];
   struct timeval start, stop;
   if (argc != 2)
   {
      printf("usage:  client <IP address>\n");
      exit(1);
   }

   sockfd=socket(AF_INET,SOCK_STREAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(32000);
   for(i=0; i < 1000; i++)
   {
      sendline[i] = 'A';
   }
   gettimeofday(&start, NULL);
   connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
   sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
   n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
   recvline[n]=0;
   gettimeofday(&stop, NULL);
   double elapsed = time_diff(start, stop);
   printf("Took: %G \n", elapsed);
   fputs(recvline,stdout);
}