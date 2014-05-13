#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#define RUN_COUNT 10
#define true 1
#define false 0
#define PSIZE 8
#define RUNTIME 200000
/*
Simulate collision when multiple devices try to transmit
over ethernet at the same time.

Each iteration of the loop is 1 51.2us slot.
If it is current somethings turn to go it will try and send.
If collision happens it will then wait 2^k-1 slots to try again
*/
typedef struct node 
{
	int initial;
	int final;
}node;

FILE *f;

int getSendCount(int array[][RUN_COUNT], int run, int currTime, int device_count) // See how many want to send
{
	int i = 0;
	int attemptingToSend = 0;
	for(i; i < device_count; i++)
		if(array[i][run] == currTime)
			attemptingToSend = attemptingToSend + 1;
	return attemptingToSend;
}
// creates a random num between 0 to 1
double rand0to1() {
    return (double)rand() / (double)RAND_MAX;
}
// returns −λ * logu, with u being a rand num between 0 to 1.
double calcRandIntervalBetweenTransmissionAttempts(int lambda) {
   return -1 * lambda * log(rand0to1());
}

int pow2(int exp) // Math library was giving issues so this calculates 2^exp
{
	return 1 << exp;
}
void collisionOccured(int array[][RUN_COUNT], int run,int currTime, int colCount, int device_count) // Handle collision occuring
{
	int i = 0;
	for(i; i < device_count; i++)
		if(array[i][run] == currTime){
			array[i][run] = currTime + 1 + (rand() % pow2(colCount)); // Add 1 since collision took a slot
		}

}

// completed - set a new time
void packetSent(int  array[][RUN_COUNT],int start[][RUN_COUNT], int run, int currTime, int lambda, int device_count)
{
	int i = 0;
	for(i; i < device_count; i++)
		if(array[i][run] == currTime) {
			
			fprintf(f,"%d\n",array[i][run] - start[i][run]);
			
			array[i][run] = currTime + PSIZE + calcRandIntervalBetweenTransmissionAttempts(lambda);
			start[i][run] = array[i][run];
		}
}
int checkFinished(int array[][RUN_COUNT], int run, int currTime, int device_count) // Check if they have all finished
{
	int i = 0;
	for(i; i < device_count; i++)
		if(array[i][run] >= currTime)
			return false;
	return true;
}
int calculateAverage(int array[][RUN_COUNT], int device) // Average out each row
{
	int i;
	int average = 0;
	for(i=0; i < RUN_COUNT; i++)
		average += array[device][i];
	return average/RUN_COUNT;

}
void sortRunsByFinishTime(int array[][RUN_COUNT], int device_count) // Sorts array by finish time (col by col)
{
	int i, j,k;
	for(k=0; k < RUN_COUNT; k++){ // Each run
    for (i = 1; i < device_count; i++) {
    	int tmp = array[i][k];
    	for (j = i; j >= 1 && tmp < array[j-1][k]; j--)
    	    	array[j][k] = array[j-1][k];
    	    array[j][k] = tmp;
    	}
    }
}
int main()
{
	time_t t;
	srand((unsigned) time(&t));
	int device_count, lambda, psize;
	printf("Enter device count: ");
	scanf("%d",&device_count);
	printf("Enter lambda: ");
	scanf("%d", &lambda);
	printf("Enter packet size: ");
	scanf("%d",&psize);
	int devices[device_count][RUN_COUNT];
	int start[device_count][RUN_COUNT];
	int colCount[RUN_COUNT];
	int runtime[RUN_COUNT];
	int completedCount[RUN_COUNT];
	int wastedSlots[RUN_COUNT];
	int i=0,j=0, colcount, num_sending, blocked, completed;
	
f = fopen("results.csv","w");
//	struct node** array = (struct node**)calloc(device_count+3,sizeof(struct node*));


	for(j=0; j < RUN_COUNT; j++) // Initialize
		for(i=0; i < device_count; i++){
			devices[i][j] = calcRandIntervalBetweenTransmissionAttempts(lambda);
			start[i][j] = devices[i][j];
			completedCount[i] = 0;
			colCount[i] = 0;
		}
	for(j = 0; j < RUN_COUNT; j++, colcount=0){
		blocked = 0;
		completedCount[j] = 0;
		wastedSlots[j] = 0;
		for(i = 0; i < RUNTIME; i++){
			num_sending = getSendCount(devices, j, i, device_count);
			if(num_sending > 0 && blocked > i) 
			{
				collisionOccured(devices,j, i, colcount, device_count);
				colcount++;
			} else if(num_sending > 1){
				collisionOccured(devices,j, i, colcount, device_count);
				colcount++;
			} else if (num_sending == 1) { // 1 is sending .. block senders for 8 timeslots
				blocked = i + psize;
				completedCount[j]++;
				packetSent(devices,start,j,i,lambda,device_count);
				colCount[j] += colcount;
				colcount = 0;
			} else 
			{
				wastedSlots[j]++;
			}
		}
	}
	fclose(f);
	for(i = 0; i < RUN_COUNT; i++){
		printf("Collision count for run %d: %d\n",i+1, colCount[i]);
		printf("Completed count for run %d: %d\n",i+1, completedCount[i]);
		printf("Total slots used: %d Total: %d\n\n", colCount[i] + completedCount[i], colCount[i]+completedCount[i]+wastedSlots[i]);
	}
}