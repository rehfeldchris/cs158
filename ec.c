#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#define DEVICE_COUNT 20
#define RUN_COUNT 20
#define true 1
#define false 0
#define PSIZE 8
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
	int colcount = 0;
	int i=0;
	int j=0;
	int num_sending;
	int lambda = 0;
	int blocked;
	printf("Enter lambda: ");
	scanf("%d", &lambda);
	for(j=0; j < RUN_COUNT; j++) // Initialize
		for(i=0; i < DEVICE_COUNT; i++){
			devices[i][j] = calcRandIntervalBetweenTransmissionAttempts(lambda);
		}
	for(j = 0; j < RUN_COUNT; j++, colcount=0, runtime[j] = 0, jitter[j] = 0, blocked = 0){
		for(i = 0; checkFinished(devices, j, i) == false; i++){
			num_sending = getSendCount(devices, j, i);
			if(num_sending > 1 || (num_sending != 0 && blocked >= i)){
				collisionOccured(devices,j, i, colcount);
				colcount++;
			} else if (num_sending == 1) { // 1 is sending .. block senders for 8 timeslots
				blocked = i + PSIZE;
			} else { // jitter occuring -- timeslot is unused
				jitter[j]++;
			}
		}
		colCount[j] = colcount;
		runtime[j] = i;
	}
	for(i=0; i < DEVICE_COUNT; i++) // Prints the average for each device
		printf("Average for device %d: %d slot wait over %d runs\n", i+1,calculateAverage(devices,i), RUN_COUNT);
	sortRunsByFinishTime(devices); // Sort by finish time
	for(i=0; i < DEVICE_COUNT; i++) // Average time to complete by order of completion
		printf("Average for place %d: %d slot wait over %d runs\n", i+1,calculateAverage(devices,i), RUN_COUNT);
	for(i = 0; i < RUN_COUNT; i++){
		printf("Run time for run %d: %d\n", i+1, runtime[i]);
		printf("Collision count for run %d: %d\n",i+1, colCount[i]);
		printf("Jitter count for run %d: %d\n", i+1, jitter[i]);
	}
}