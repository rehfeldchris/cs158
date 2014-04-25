#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

time_t t;
int numLambdas = 9;
int maxLambdaVal = 20;
int lambdas[10] = {20, 18, 16, 14, 12, 10, 8, 6, 4};
int numTrialsPerLambda = 50;
int results[21][51];

double rand0to1() {
    return (double)rand() / (double)RAND_MAX;
}

double calcRandIntervalBetweenTransmissionAttempts(int lambda) {
	return -1 * lambda * log(rand0to1());
}

int isCollision(double timeNow, double currentAttemptTime) {
	return (timeNow + 1) > currentAttemptTime && (timeNow - 1) < currentAttemptTime;
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

int calcContentionInterval(int lambda) {
	int i, numNonCollisions = 0, numAttempts = 0;
	double interval, currentAttemptTime, timeNow = 0;

	for (i = 0; i < 20; i++) {
		interval = calcRandIntervalBetweenTransmissionAttempts(lambda);
		printf("%.3f\n", interval);
		currentAttemptTime = interval + timeNow;
		//printf("%.5lf %.5lf %.5lf\n", timeNow , interval , currentAttemptTime);

		if (!isCollision(timeNow, currentAttemptTime)) {
			numNonCollisions++;
		}

		numAttempts++;
		timeNow = currentAttemptTime;

		if (numNonCollisions == 2) {
			return numAttempts;
		}
	}

	// shouldnt happen
	return -100000;
}

// calcs the number of attempts we had to use to succeed at transmitting without a collision, for each possible lambda.
// does it 50 times for each lambda, storing the results in an array.
void runSim() {
	int i, j, lambda;
	for (i = 0; i < numLambdas; i++) {
		lambda = lambdas[i];
		for (j = 0; j < numTrialsPerLambda; j++) {
			results[lambda][j] = calcContentionInterval(lambda);
			printf(
				"lambda = %d, attempts = %d\n",
				lambda,
				results[lambda][j]
			);
		}
	}
}

void printAverages() {
	int i, j, lambda, attempts;
	for (i = 0; i < numLambdas; i++) {
		lambda = lambdas[i];
		attempts = 0;
		for (j = 0; j < numTrialsPerLambda; j++) {
			attempts += results[lambda][j];
		}
		printf(
			"lambda = %d, avg = %.3f\n",
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
