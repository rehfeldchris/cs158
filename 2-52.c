#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_LAMBDAS 9

time_t t;
int maxLambdaVal = 20;
int lambdas[NUM_LAMBDAS] = {20, 18, 16, 14, 12, 10, 8, 6, 4};
int numTrialsPerLambda = 50;
int results[21][51];
double interval, currentAttemptTime, prev1AttemptTime, prev2AttemptTime, timeNow = 0;

/*
 * Code for ch2 problem 52
 * project 3 part c
 **/

// creates a random num between 0 to 1
double rand0to1() {
    return (double)rand() / (double)RAND_MAX;
}

// returns −λ * logu, with u being a rand num between 0 to 1.
double calcRandIntervalBetweenTransmissionAttempts(int lambda) {
	return -1 * lambda * log(rand0to1());
}

// inits the results multidim array to all zeros
void initResultsArray() {
	int i, j;
	for (i = 0; i < maxLambdaVal; i++) {
		for (j = 0; j < numTrialsPerLambda; j++) {
			results[i][j] = 0;
		}
	}
}

// checks if the previous(not the most recent) transmission attempt
// collided with the current, or the "previous previous"
int isPrevAttemptACollision() {
	return (timeNow + 1) > prev1AttemptTime && (timeNow - 1) < prev1AttemptTime;
}

// simulates a transmission, updating global bookeeping variables
void makeTransmissionAttempt(int lambda) {
	// shift the attempt time records backwards in our history buffers
	prev2AttemptTime = prev1AttemptTime;
	prev1AttemptTime = currentAttemptTime;

	// make a new attempt
	interval = calcRandIntervalBetweenTransmissionAttempts(lambda);
	currentAttemptTime = interval + timeNow;
	timeNow = currentAttemptTime;
}

// calculates how long was needed before a successful transmission occured.
// the units are slot times.
int calcContentionInterval(int lambda) {
	int numAttempts;

	// we start by making 2 attempts. We do this because only on or after the 3rd attempt would it be
	// possible to identify a successful transmission. It's possible the 2nd transmission was successful.
	makeTransmissionAttempt(lambda);
	makeTransmissionAttempt(lambda);
	numAttempts = 2;

	do {
		makeTransmissionAttempt(lambda);
		numAttempts++;
	} while (isPrevAttemptACollision());

	// we always subtract 1 because it was the previous attempt that succeeded. we just didnt
	// know that it succeeded until after we had made another attempt.
	return numAttempts - 1;
}


// calcs the number of slot times we had to use to succeed at transmitting without a collision, for each possible lambda.
// it does it 50 times for each lambda, storing the results in an array.
void runSim() {
	int i, j, lambda;
	printf("trial#\tlambda\tslot times\n");
	for (i = 0; i < NUM_LAMBDAS; i++) {
		lambda = lambdas[i];
		for (j = 0; j < numTrialsPerLambda; j++) {
			results[lambda][j] = calcContentionInterval(lambda);
			printf(
				"%d\t%d\t%d\n",
				j,
				lambda,
				results[lambda][j]
			);
		}
	}
}

// computes and prints the average statistics
void printAverages() {
	int i, j, lambda, attempts;
	printf("----Averages of runs----\n");
	printf("lambda\tavg slot time\n");
	for (i = 0; i < NUM_LAMBDAS; i++) {
		lambda = lambdas[i];
		attempts = 0;
		for (j = 0; j < numTrialsPerLambda; j++) {
			attempts += results[lambda][j];
		}
		printf(
			"%d\t%.3f\n",
			lambda,
			attempts / (double) numTrialsPerLambda
		);
	}
}

int main(int argc, char * argv[]) {
	srand((unsigned) time(&t));
	runSim();
	printAverages();

	return 0;
}
