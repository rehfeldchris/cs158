#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT 5432
#define MAX_LINE 64000



struct sockaddr_in sin;
FILE *fp;
struct hostent *hp;
char *host;
char buf[MAX_LINE + 1];
int s;
int len;
int bytes;
int msg_len = 10;
int num_messages_to_send_per_size = 100;

struct timeval start, stop;

// returns 1 unless a send or recv failed to work perfect
int send_and_receive_msg(size_t len) {
	int bytes;
	bytes = sendto(s, buf, len, 0, (struct sockaddr *) &sin, sizeof(sin));

	if (len != bytes) {
		printf("tried to send %d but got %d\n", len, bytes);
		return 0;
	}

	bytes = recvfrom(s, buf, sizeof(buf), 0, NULL, 0);
	// only count them if its the full size were expecting, otherwise its a malformed packet, or -1 for timeout
	if (len != bytes) {
		printf("tried to receive %d but got %d\n", len, bytes);
		return 0;
	}

	return 1;
}


// sometimes we after we move onto doing messages for the next larger message size, we get
// a message for the prev size. the packet was prob on its way back when we switched message sizes.
// so this func tries to collect any it finds.
int recv_any_remaining_messages(size_t len) {
	int bytes, num_found_msgs = 0;
	do {
		bytes = recvfrom(s, buf, sizeof(buf), 0, NULL, 0);
		// only count them if its the full size were expecting, otherwise its a malformed packet, or -1 for timeout
		if (bytes == len) {
			num_found_msgs++;
		}
	} while(bytes != -1);
	return num_found_msgs;
}

void init_msg(int len) {
	int i;
	// make a msg of all a's
	for (i = 0; i < len - 1; i++) {
		buf[i] = 'a';
	}
	buf[msg_len - 1] = '\0';
}


double time_diff(struct timeval x , struct timeval y)
{
	return (y.tv_sec - x.tv_sec) + 1e-6 * (y.tv_usec - x.tv_usec);
}

// we need to use socket read timeouts otherwise we block forver waiting for recv() to return, if a message gets lost.
// also, make the timeout proportional to expected message size
void set_socket_timeout_for_message_size(int bytes) {
	// 10ms + extra based on size
	int usec = 10000 + bytes*10;
	struct timeval recv_timeout;
	recv_timeout.tv_sec = 0;
	recv_timeout.tv_usec  = usec;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeout, sizeof(struct timeval));
}





int main(int argc, char * argv[]) {
	int i, j;
	int msg_sizes[10] = {1, 1024, 4*1024, 8*1024, 16*1024, 32*1024, 64000 };
	int num_msg_sizes = 7;
	printf(argv[1]);
	if (argc == 2) {
		host = argv[1];
	} else {
		fprintf(stderr, "usage: simplex-talk-client host\n");
		exit(1);
	}
	/* translate host name into peer's IP address */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "simplex-talk-client: unknown host: %s\n", host);
		exit(1);
	}

	/* build address data structure */
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(SERVER_PORT);
	/* active open */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("simplex-talk-client: socket\n");
		exit(1);
	}


	for (i = 0; i < num_msg_sizes; i++) {
		msg_len = msg_sizes[i];
		init_msg(msg_len);
		set_socket_timeout_for_message_size(msg_len);
		gettimeofday(&start, NULL);
		int num_successful_messages = 0;
		for (j = 0; j < num_messages_to_send_per_size; j++) {
			int success = send_and_receive_msg((size_t) msg_len);
			if (success) {
				num_successful_messages++;
			}
		} 
		int lost_and_found = recv_any_remaining_messages((size_t) msg_len);
		if (lost_and_found) {
			num_successful_messages += lost_and_found;
			printf("found %d %dB messages lingering afterwards\n", lost_and_found, msg_len);
		}
		
		gettimeofday(&stop, NULL);
		double elapsed = time_diff(start, stop);

		printf(
			"msg size=%dB, attempted=%d, successful=%d, total time=%.4f, avg rtt=%.4f, throughput=%.1fb per sec\n",
			msg_len,
			num_messages_to_send_per_size,
			num_successful_messages,
			elapsed,
			elapsed / num_messages_to_send_per_size,
			8 * (num_successful_messages *msg_len ) / elapsed
		);  
	}

//	getc(stdin);
}



