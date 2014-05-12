#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#define PORT 5000
#define MAX_BYTE 65537
#define SIZE_COUNT 7
#define SEND_COUNT 100
#define SLOT_TIME_MICROSECONDS 800
#define SLOT_TIME_NANOSECONDS 800000
#define RUNTIME_IN_SLOTS 5000
#define NUM_LAMBDAS 9

int sock, numConsecutiveCollisions;
struct timeval timeStartedSendingMessages, timeMostRecentMessageSent;
int lambdas[NUM_LAMBDAS] = {20, 18, 16, 14, 12, 10, 8, 6, 4};

double time_diff(struct timeval x , struct timeval y) {
   return (y.tv_sec - x.tv_sec) + 1e-6 * (y.tv_usec - x.tv_usec);
}

long long diff_microseconds(struct timeval x , struct timeval y) {
	time_t diff_secs = x.tv_sec - y.tv_sec;
	time_t diff_microsecs = x.tv_usec - y.tv_usec;
	return diff_microsecs + diff_secs * 1000000LL;
}

// this will look at the current clock time and sleep a certain amount of time
// so that it will wake up at the next multiple of 2 seconds. eg, if the time now is
// 4:52:33 pm then this func will sleep until 4:52:35 pm.
// this is done so that you can start multiple instances of the client program, and have them all start actually
// sending at about the same time.
void synchronizeWithOtherClients() {
	struct timeval now;
	struct timespec sleepDuration, unsleptRemainder;
	gettimeofday(&now, NULL);

	// first sleep for 1/10 second uncondtionally. this
	// ensures the other clients finish up anything they might be working on.
	sleepDuration.tv_sec = 0;
	sleepDuration.tv_nsec = 100000000L;
	//printf("synchronizing with other clients...sleeping for %lld seconds, %lu nanoseconds\n", (long long)sleepDuration.tv_sec, sleepDuration.tv_nsec);
	nanosleep(&sleepDuration, &unsleptRemainder);

	// now sleep until the next multiple of time in order
	// to synchronize with other clients.
	sleepDuration.tv_sec = 2 - (now.tv_sec % 2);
	sleepDuration.tv_nsec = 1000000000L - (now.tv_usec * 1000);
	//printf("synchronizing with other clients...sleeping for %lld seconds, %lu nanoseconds\n", (long long)sleepDuration.tv_sec, sleepDuration.tv_nsec);
	nanosleep(&sleepDuration, &unsleptRemainder);
}

long mypow(int base, int exponent) {
	int pow = base;
	if (exponent == 0) {
		return 1;
	}
	while (--exponent > 0) {
		pow *= base;
	}
	return pow;
}

long exponentialBackoffTime(int k) {
	return SLOT_TIME_NANOSECONDS * (rand() % (mypow(2, k) - 1));
}

// sleeps the normal interval we use between messages, or maybe longer if collisions occurred recently
void sleepBetweenMessages(int lambda) {
	struct timespec sleepDuration, unsleptRemainder;
	sleepDuration.tv_sec = 0;

	if (numConsecutiveCollisions > 0) {
		sleepDuration.tv_nsec = lambda * exponentialBackoffTime(numConsecutiveCollisions);
	} else {
		// we sleep for lambda slots even when no collisions
		sleepDuration.tv_nsec = lambda * SLOT_TIME_NANOSECONDS;
	}

	nanosleep(&sleepDuration, &unsleptRemainder);
}

// how many time slots elapsed since we started sending messages
int getNumElapsedTimeSlots() {
	struct timeval now;
	gettimeofday(&now, NULL);
	long long elapsed = diff_microseconds(now, timeStartedSendingMessages);
	return elapsed / SLOT_TIME_MICROSECONDS;
}

int shouldKeepSendingMessages() {
	return getNumElapsedTimeSlots() < RUNTIME_IN_SLOTS;
}



// returns 1 if the server said it was successful, 0 if server said a collision occurred.
int sendAndWaitForReply() {
	char * msg = "A";
	char buf[2];
	int bytesSent, bytesReceived, msgLen;

	//printf("sending msg\n");

	msgLen = strlen(msg);
	bytesSent = send(sock, msg, msgLen, 0);
	if (bytesSent < msgLen) {
		perror("send");
		exit(1);
	}

	//printf("awaiting reply\n");

	memset(buf, 0, 2);
	bytesReceived = recv(sock, buf, 1, 0);
	if (bytesReceived < 1) {
		perror("recv");
		exit(1);
	}

	//printf("got back '%s'\n", buf);

	if (strcmp(buf, "S") == 0) {
		numConsecutiveCollisions = 0;
		return 1;
	} else if (strcmp(buf, "C") == 0) {
		numConsecutiveCollisions++;
		return 0;
	} else {
		perror("got bad reply");
		exit(1);
	}
}

void startSendingMessagesToServer() {
	int i, lambda, numSuccess, numCollision;

	printf("timeslots per lambda: %d\n", RUNTIME_IN_SLOTS);

	// print an ascii table header
	printf("lambda\tload\tsent\tsuccess\tcollision\tdelay\tthroughput\tpercent success\n");

	// for each lambda size, send messages to the server and record/print statistics
	for (i = 0; i < NUM_LAMBDAS; i++) {
		lambda = lambdas[i];
		synchronizeWithOtherClients();

		numSuccess = numCollision = 0;
		gettimeofday(&timeStartedSendingMessages, NULL);

		while (shouldKeepSendingMessages()) {
			sendAndWaitForReply() ? numSuccess++ : numCollision++;
			sleepBetweenMessages(lambda);
		}

		// print a table row
		printf(
			"%d\t%.5f\t%d\t%d\t%d\t%.4f\t\t%.5f\t\t%.1f\n",
			lambda,
			2 / (float) lambda,
			numSuccess + numCollision,
			numSuccess,
			numCollision,
			RUNTIME_IN_SLOTS - (numSuccess * lambda),
			numSuccess / (float) RUNTIME_IN_SLOTS,
			(100 * numSuccess) / (float) (numSuccess + numCollision)
		);
	}
}




void connectToServer() {
	struct sockaddr_in server_addr;
	char *hostname = "127.0.0.1";
	struct hostent *host = gethostbyname(hostname);

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("Socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero), 8);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
    	perror("Connect");
    	exit(1);
    }
}

int main(int argc, char * argv[]) {
	srand(time(NULL));
	connectToServer();
	startSendingMessagesToServer();
	return 0;
}
