#include <iostream>
#include <cmath>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <array>
#include <float.h>
#include <sstream>

// to avoid magic numbers
const static unsigned int DIMENSIONS = 2; // how many dimensions
const static unsigned int NEIGHBORS = 4; // how many moves to make per position
const static unsigned int MIN_THREADS = 1, MAX_THREADS = 8; // range for threading

// globals
std::array<double, DIMENSIONS> bestPosition; // best position so far
double bestResult = DBL_MAX; // arbitrarily large
volatile unsigned int occupied; // for thread count
volatile bool continuing; // for thread while loop
pthread_mutex_t updateLock; // lock
unsigned int functionType = 1; // what type of function to use
double bound = 512.00; // bounds for function
double rBound = bound * 0.10;

// function declarations
bool checkInBounds(std::array<double, DIMENSIONS>, int, int);
std::array<double, DIMENSIONS> getRandPosition(int, int);
std::string printBest(); 
double eggHolderFunction(std::array<double, DIMENSIONS>);
double sphereFunction(std::array<double, DIMENSIONS>);
double griewankFunction(std::array<double, DIMENSIONS>);
double rastriginFunction(std::array<double, DIMENSIONS>);
double schwefelFunction(std::array<double, DIMENSIONS>);
double dixonPriceFunction(std::array<double, DIMENSIONS>);
double sumSquaresFunction(std::array<double, DIMENSIONS>);
double sumDifferentPowersFunction(std::array<double, DIMENSIONS>);

/**
 * fitness evaluation driver
 * @param p - the position to evaluate
 * @return - the fitness
 */
double fitnessEvaluation(std::array<double, DIMENSIONS> p) {
	switch(functionType) {
		case 1: return eggHolderFunction(p);
		case 2: return schwefelFunction(p);
		case 3: return rastriginFunction(p);
		case 4: return griewankFunction(p);
		case 5: return sphereFunction(p);
		case 6: return dixonPriceFunction(p);
		case 7: return sumSquaresFunction(p);
		case 8: return sumDifferentPowersFunction(p);
		default: return 0.00; // should never get here
	}
}

/**
 * Sum of Different Powers evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position
 */
double sumDifferentPowersFunction(std::array<double, DIMENSIONS> p) {
	double sum = 0.00;
	for (unsigned int i = 0; i < DIMENSIONS; i++) {
		sum += std::pow(std::abs(p[i]), (i+1));
	}
	return sum;
}

/**
 * Sum Squares evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position
 */
double sumSquaresFunction(std::array<double, DIMENSIONS> p) {
	double sum = 0.00;
	for (unsigned int i = 0; i < DIMENSIONS; i++) {
		sum += (i+1) * std::pow(p[i],2);
	}
	return sum;
}

/**
 * Dixon-Price evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position
 */
double dixonPriceFunction(std::array<double, DIMENSIONS> p) {
	double sum = std::pow((p[0] - 1), 2);
	for (unsigned int i = 1; i < DIMENSIONS; i++) {
		sum += (i+1) * std::pow((2*std::pow(p[i],2) - p[i-1]),2);
	}
	return sum;
}

/**
 * Sphere evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position
 */
double sphereFunction(std::array<double, DIMENSIONS> p) {
	double sum = 0.00;
	for (unsigned int i = 0; i < DIMENSIONS; i++) {
		sum += std::pow(p[i],2);
	}
	return sum;
}

/**
 * Griewank evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position (lower is better)
 */
double griewankFunction(std::array<double, DIMENSIONS> p) {
	double sum = 0.00; double product = 1.00;
	for (unsigned int i = 0; i < DIMENSIONS; i++) {
		sum += (std::pow(p[i],2) / 4000);
		product *= (std::cos(p[i] / std::sqrt(i+1)));
	}
	return (sum - product + 1);
}

/**
 * Rastrigin evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position (lower is better)
 */
double rastriginFunction(std::array<double, DIMENSIONS> p) {
	double sum = 10 * DIMENSIONS;
	for (unsigned int i = 0; i < DIMENSIONS; i++) {
		sum += (std::pow(p[i],2) - (10 * std::cos(2*M_PI * p[i])));
	}
	return sum;
}

/**
 * Schwefel evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position (lower is better)
 */
double schwefelFunction(std::array<double, DIMENSIONS> p) {
	double sum = 418.9829 * DIMENSIONS;
	double subSum = 0.00;
	for (unsigned int i = 0; i < DIMENSIONS; i++) {
		subSum += (p[i] * std::sin(std::sqrt(std::abs(p[i]))));
	}
	return sum - subSum;
}

/**
 * Egg Holder evaluation function
 * @param p - the position to evaluate
 * @return - the fitness of that position (lower is better)
 */
double eggHolderFunction(std::array<double, DIMENSIONS> p) {
	double sum = 0.00;
	for (unsigned int i = 0; i < DIMENSIONS-1; i++) {
		// split for readability, this is the EH summation
		double subSum = (p[i+1] + 47) * (std::sin(std::sqrt(std::abs((p[i]/2) + p[i+1] + 47))));
		subSum -= (p[i] * std::sin(std::sqrt(std::abs(p[i] - p[i+1] - 47))));
		subSum *= (-1);
		sum += subSum;
	}
	return sum;
}

/**
 * Hill Climbing worker function, one per thread
 */
void* hillClimb(void* ignore) {
	std::array<double, DIMENSIONS> position; // position for evaluation later
	std::array<double, DIMENSIONS> stoch; // stochastic element
	double best, tempBest; // best of thread, temporary best used later
	while (continuing) { // while thread alive
		position = getRandPosition(-bound, bound); // find a random position -512..512 for each dimension
		best = fitnessEvaluation(position); // find fitness of that position
		while (checkInBounds(position, -bound, bound)) { // while the position is within bounds
			for (unsigned int i = 0; i < NEIGHBORS; i++) { // four possible moves
				stoch = getRandPosition(-rBound, rBound); // stochastic summand position -5..5
				std::array<double, DIMENSIONS> tempPos; // for adding
				// add the stochastic element to the position
				for (unsigned int j = 0; j < DIMENSIONS; j++) { tempPos[j] = stoch[j] + position[j]; }
				tempBest = fitnessEvaluation(tempPos); // evaluate that new position
				if (tempBest < best) { // if better than loop
					position = tempPos; // overwrite
					best = tempBest;
				}
			}
			// only one lock is needed as output is performed as updates are made
			pthread_mutex_lock(&updateLock);
			if (best < bestResult) { // if better than global
				bestPosition = position; // overwrite
				bestResult = best;
				std::cout << "New minimum: " << printBest();
			}
			pthread_mutex_unlock(&updateLock); // unlock
		}
	}
	occupied--; // note number of active threads
	return 0;
}

/**
 * function to find a random position
 * @param l - lower bound
 * @param h - higher bound
 * @return - a random position in all dimensions l..h
 */
std::array<double, DIMENSIONS> getRandPosition(int l, int h) {
	std::array<double, DIMENSIONS> p;
	for (unsigned int i = 0; i < DIMENSIONS; i++) { // for each dimension
		// generate random position
		p[i] = l + (double)(rand() / (double)RAND_MAX) * (h - l);
	}
	return p;
}

/**
 * function to check if a position is within bounds
 * @param p - the position to check
 * @param l - lower bound
 * @param h - higher bound
 * @return - whether p is within l..h
 */
bool checkInBounds(std::array<double, DIMENSIONS> p, int l, int h) {
	for (unsigned int i = 0; i < DIMENSIONS; i++) { // for each dimension
		// first dimension proc chance may come earlier; return F on per dimension basis
		if (p[i] > h || p[i] < l) { return false; }
	}
	// if it gets here, it's all within bounds
	return true;
}

/**
 * function to print the best to user
 * @return - the function with params and its evaluation
 */
std::string printBest() {
	std::stringstream toReturn;
	toReturn << "f(";
	for (unsigned int i = 0; i < DIMENSIONS; i++) {
		toReturn << bestPosition[i];
		if (i < DIMENSIONS - 1) { toReturn << ", "; }
	}
	toReturn << ") = " << bestResult << "\n";
	return toReturn.str().c_str();
}

// interrupt handler
void interrupted(int signal) {
    continuing = false;
}

// check best handler
void checkBest(int signal) {
	std::cout << "Best so far: " << printBest();
}

// entry point
int main (int argc, char* argv[]) {
	srand(time(NULL)); // initialize random seed based on system time
	signal(SIGINT, interrupted); // signal handler overrides
	signal(SIGUSR1, checkBest);
	pthread_t threads[MAX_THREADS]; // max of eight threads
	if (argc != 3) { std::cout << "Expected two arguments." << std::endl; return 1; }
	unsigned int choiceA = atoi(argv[1]); // parse user input for number of climbers
	unsigned int choiceB = atoi(argv[2]); // parse user input for function type
	if (choiceA > MAX_THREADS) { std::cout << "Too many climbers." << std::endl; return 1; }
	if (choiceA < MIN_THREADS) { std::cout << "Too few climbers." << std:: endl; return 1; }
	switch(choiceB) {
		case 1: bound = 512.00; break; // egg holder
		case 2: bound = 500.00; break; // schwefel
		case 3: bound = 5.12; break; // rastrigin
		case 4: bound = 600.00; break; // griewank
		case 5: bound = 5.12; break; // sphere
		case 6: bound = 10; break; // dixon-price
		case 7: bound = 10; break; // sum squares
		case 8: bound = 1; break; // sum of different powers
		default: std::cout << "Invalid function type." << std::endl; return 1; // invalid
	}
	functionType = choiceB; // init the function type
	rBound = bound*0.10; // stochastic jump is 10% of bound
	continuing = true;
	for (unsigned int i = 0; i < choiceB; i++) {
		pthread_create(&threads[i], NULL, &hillClimb, NULL);
		occupied++;
	}
	while (continuing && occupied > 0) { sleep(1); }
	std::cout << "\nBest in run: " << printBest();
    return 0;
}
