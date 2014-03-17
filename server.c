#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5432
#define MAX_LINE    64000

int main() {
	struct sockaddr_in sin;
	struct sockaddr_in remote;
	int remote_len;
	char buf[MAX_LINE + 1];
	int len;
	int s;
	int bytes;

	/* build address data structure */
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);

	/* setup passive open */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}
	if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind");
		exit(1);
	}

	remote_len = sizeof(remote);
	while (1) {
		while (bytes = recvfrom(s, buf, sizeof(buf), 0,	(struct sockaddr *) &remote, &remote_len)) {
		    //printf("Got a datagram from %s port %d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
			//fputs(buf, stdout);
			printf("server bytes received: %d\n", bytes);
			bytes = sendto(s, buf, bytes, 0, (struct sockaddr *) &remote, remote_len);
		}
		close(s);
	}
}
