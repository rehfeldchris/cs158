#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#define DEVICE_COUNT 5
#define RUN_COUNT 100	
#define true 1
#define false 0
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
int pow2(int exp) // Math library was giving issues so this calculates 2^exp
{
	int i = 0;
	int total = 2;
	if(exp == 0)
		return 1;
	for(i; i < exp; i++)
		total *= 2;
	return total;
}
void collisionOccured(int array[DEVICE_COUNT][RUN_COUNT], int run,int currTime, int colCount) // Handle collision occuring
{
	int i = 0;
	for(i; i < DEVICE_COUNT; i++)
		if(array[i][run] == currTime)
			array[i][run] = currTime+ 1 + (rand() % pow2(colCount)); // Add 1 since collision took a slot
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
	srand(time(NULL));
	int devices[DEVICE_COUNT][RUN_COUNT];
	int colcount = 0;
	int i=0;
	int j=0;
	for(j=0; j < RUN_COUNT; j++) // Initialize
		for(i=0; i < DEVICE_COUNT; i++)
			devices[i][j] = 0;
	for(j = 0; j < RUN_COUNT; j++, colcount=0)
		for(i = 0; checkFinished(devices, j, i) == false; i++)
			if(getSendCount(devices,j, i) > 1){
				collisionOccured(devices,j, i, colcount);
				colcount++;
			}
	for(i=0; i < DEVICE_COUNT; i++) // Prints the average for each device
		printf("Average for device %d: %d slot wait over %d runs\n", i+1,calculateAverage(devices,i), RUN_COUNT);
	sortRunsByFinishTime(devices); // Sort by finish time
	for(i=0; i < DEVICE_COUNT; i++) // Average time to complete by order of completion
		printf("Average for place %d: %d slot wait over %d runs\n", i+1,calculateAverage(devices,i), RUN_COUNT);
}