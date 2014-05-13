#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_BUF 65537
#define PORT 5000
#define NUM_CLIENTS 2
#define MAX_CONNECTIONS 2
#define SLOT_TIME_MICROSECONDS 800
#define SLOT_TIME_NANOSECONDS 800000

pthread_t threads[MAX_CONNECTIONS];
int sock, connected, bytes_recieved , true = 1;
char send_data[MAX_BUF], recv_data[MAX_BUF];

struct sockaddr_in server_addr,client_addr;
int sin_size;
struct timeval mostRecentMessage;
pthread_mutex_t messageRecordingLock;
int numMessagesReceived = 0;

typedef struct {
    int conn;
    size_t sin_size;
    struct sockaddr_in client_addr;
} ConnectionInfo;

void init() {
    // we initialize these values so that the arithmetic which checks
    // for a collision will work properly on the first message.
    mostRecentMessage.tv_sec = 0;
    mostRecentMessage.tv_usec = 0;

    if (pthread_mutex_init(&messageRecordingLock, NULL) != 0) {
        printf("\n mutex init failed\n");
        exit(1);
    }
}

long long diff_microseconds(struct timeval x , struct timeval y) {
    time_t diff_secs = x.tv_sec - y.tv_sec;
    time_t diff_microsecs = x.tv_usec - y.tv_usec;
    return diff_microsecs + diff_secs * 1000000LL;
}

int atLeast1SlotTimeElapsedSincePrevMessage() {
    struct timeval now;
    gettimeofday(&now, NULL);
    long long elapsed = diff_microseconds(now, mostRecentMessage);
    //printf("elapsed: %lld\n", elapsed);
    return elapsed > SLOT_TIME_MICROSECONDS;
}

void markMessageReceived() {
    gettimeofday(&mostRecentMessage, NULL);
    numMessagesReceived++;
}

void sleepFor1SlotTime() {
    struct timespec sleepDuration, unsleptRemainder;
    //printf("sleeping\n");
    sleepDuration.tv_sec = 0;
    sleepDuration.tv_nsec = SLOT_TIME_NANOSECONDS;
    nanosleep(&sleepDuration, &unsleptRemainder);
}

// returns 0 if it thinks the client disconnected/errored, else 1
int receivedAndReply(ConnectionInfo * connectionInfoPtr) {
    char buf[MAX_BUF];
    char * msg;
    int bytesReceived, bytesSent, isCollision, len, numMessagesBeforeSleep;

    memset(buf, 0, MAX_BUF);

    //we assume the client always sends 1 byte messages
    bytesReceived = recv(connectionInfoPtr->conn, buf, MAX_BUF, 0);
    if (bytesReceived < 1) {
        perror("recv");
        return 0;
    }
    //printf("got %d bytes '%s'\n", bytesReceived, buf);

    // to check for collision, we start by looking if a previous message was received within
    // 1 slot time ago. if so, its a collsion for sure. but, if not, then we note how many messages the server has received
    // at this instant in time, and then we goto sleep for 1 slot. when we wake up, we again look at how many messages
    // the server has received, and if it changed, that means a new message came while we were sleeping. this means a collision.
    // otherwise, the message was successfully received.
    pthread_mutex_lock(&messageRecordingLock);
    numMessagesBeforeSleep = numMessagesReceived;
    isCollision = !atLeast1SlotTimeElapsedSincePrevMessage();
    markMessageReceived();
    pthread_mutex_unlock(&messageRecordingLock);

    if (isCollision) {
        //printf("collision before sleep\n");
    }

    if (!isCollision) {
        sleepFor1SlotTime();
        pthread_mutex_lock(&messageRecordingLock);
        // we subtract 1 because we added our own message
        isCollision = numMessagesBeforeSleep != numMessagesReceived - 1;
        pthread_mutex_unlock(&messageRecordingLock);
        if (isCollision) {
            //printf("collision after sleep\n");
        }
    }

    msg = isCollision ? "C" : "S";

    len = strlen(msg);
    bytesSent = send(connectionInfoPtr->conn, msg, len, 0);
    if (bytesSent < 1) {
        perror("send");
        return 0;
    }

    //printf("sent reply\n");
    return 1;
}

void *serviceClient(void * param) {
    ConnectionInfo * connectionInfoPtr = (ConnectionInfo *) param;

    while(receivedAndReply(connectionInfoPtr)) {}

    printf("thread ending\n");
    return NULL;
}

void acceptAndServiceConnection() {
    pthread_t thread;
    ConnectionInfo * connectionInfoPtr = (ConnectionInfo *) malloc(sizeof(ConnectionInfo));
    connectionInfoPtr->sin_size = sizeof(struct sockaddr_in);
    connectionInfoPtr->conn = accept(sock, (struct sockaddr *)&connectionInfoPtr->client_addr, &connectionInfoPtr->sin_size);

    if (connectionInfoPtr->conn == -1) {
        perror("accept failed");
    } else {
        printf("\nCONNECTION: (%s , %d)\n", inet_ntoa(connectionInfoPtr->client_addr.sin_addr), ntohs(connectionInfoPtr->client_addr.sin_port));
    }

    if (pthread_create(&thread, NULL, serviceClient, connectionInfoPtr)) {
        perror("Error creating thread\n");
        exit(1);
    }

    printf("spawned thread\n");
}


void prepareServerSocketForClients() {
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(sock, 5) == -1) {
        perror("Listen");
        exit(1);
    }

    printf("\nTCPServer Waiting for clients on port %d\n", PORT);
}


int main(int argc, char * argv[]) {
    prepareServerSocketForClients();
    while(1) {
        acceptAndServiceConnection();
    }
}
