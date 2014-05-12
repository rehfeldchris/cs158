#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#define DEVICE_COUNT 20
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

int getSendCount(int array[DEVICE_COUNT][RUN_COUNT], int run, int currTime) // See how many want to send
{
	int i = 0;
	int attemptingToSend = 0;
	for(i; i < DEVICE_COUNT; i++)
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
void collisionOccured(int array[DEVICE_COUNT][RUN_COUNT], int run,int currTime, int colCount) // Handle collision occuring
{
	int i = 0;
	for(i; i < DEVICE_COUNT; i++)
		if(array[i][run] == currTime){
			array[i][run] = currTime + 1 + (rand() % pow2(colCount)); // Add 1 since collision took a slot
		}

}

// completed - set a new time
void packetSent(int  array[DEVICE_COUNT][RUN_COUNT], int run, int currTime, int lambda)
{
	int i = 0;
	for(i; i < DEVICE_COUNT; i++)
		if(array[i][run] == currTime)
			array[i][run] = currTime + PSIZE + calcRandIntervalBetweenTransmissionAttempts(lambda);
}
int checkFinished(int array[DEVICE_COUNT][RUN_COUNT], int run, int currTime) // Check if they have all finished
{
	int i = 0;
	for(i; i < DEVICE_COUNT; i++)
		if(array[i][run] >= currTime)
			return false;
	return true;
}
int calculateAverage(int array[DEVICE_COUNT][RUN_COUNT], int device) // Average out each row
{
	int i;
	int average = 0;
	for(i=0; i < RUN_COUNT; i++)
		average += array[device][i];
	return average/RUN_COUNT;

}
void sortRunsByFinishTime(int array[DEVICE_COUNT][RUN_COUNT]) // Sorts array by finish time (col by col)
{
	int i, j,k;
	for(k=0; k < RUN_COUNT; k++){ // Each run
    for (i = 1; i < DEVICE_COUNT; i++) {
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
	int devices[DEVICE_COUNT][RUN_COUNT];
	int jitter[RUN_COUNT];
	int colCount[RUN_COUNT];
	int runtime[RUN_COUNT];
	int completedCount[RUN_COUNT];
	int i=0,j=0, colcount, lambda, num_sending, blocked, completed;
	printf("Enter lambda: ");
	scanf("%d", &lambda);
	for(j=0; j < RUN_COUNT; j++) // Initialize
		for(i=0; i < DEVICE_COUNT; i++){
			devices[i][j] = calcRandIntervalBetweenTransmissionAttempts(lambda);
			completedCount[i] = 0;
			colCount[i] = 0;
		}
	for(j = 0; j < RUN_COUNT; j++, colcount=0){
		jitter[j] = 0;
		blocked = 0;
		completedCount[j] = 0;
		for(i = 0; i < RUNTIME; i++){
			num_sending = getSendCount(devices, j, i);
			if(num_sending > 0 && blocked > i) // Jitter?
			{
				collisionOccured(devices,j, i, colcount);
				colcount++;
				jitter[j]++;
			} else if(num_sending > 1){
		//		printf("collision occuring in run #%d at time %d\n",j,i);
	//			printf("num_sending = %d blocked = %d\n",num_sending,blocked);
				collisionOccured(devices,j, i, colcount);
				colcount++;
			} else if (num_sending == 1) { // 1 is sending .. block senders for 8 timeslots
				blocked = i + PSIZE;
				completedCount[j]++;
				packetSent(devices,j,i,lambda);
		//		printf("Blocked until %d\n",blocked);
				colCount[j] += colcount;
				colcount = 1;
			} else { // jitter occuring? -- timeslot is unused
			//	jitter[j]++;
			}
		}
		
		runtime[j] = i;
	}
	for(i = 0; i < RUN_COUNT; i++){
		printf("Collision count for run %d: %d\n",i+1, colCount[i]);
		printf("Jitter count for run %d: %d\n", i+1, jitter[i]);
		printf("Completed count for run %d: %d\n\n",i+1, completedCount[i]);
	}
}