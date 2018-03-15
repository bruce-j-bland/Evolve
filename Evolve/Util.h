#pragma once

#include <math.h>
#include <string>
#include <iostream>
#include <time.h>
#include <random>
#include <algorithm> 
#include <numeric>
#include <map>
#include "PerlinNoise.h"

using namespace std;

//Used for optimization in generating an image to avoid repeated work
//Not precomputed, precomputed constant, precomputed with respect to x
enum prestat { no, con, xVar };

// Converts the decimal part of doubles to ints to approximate bitwise operations
int ratioToBin(double x);

// Converts ints back to doubles
double binToRatio(int x);

// Approimating bitwise functions. Only deals with decimal parts of doubles
// Returns values between 0 and 1
double ratioAnd(double a, double b);

double ratioOr(double a, double b);

double ratioXor(double a, double b);


// Returns if two strings are equal, case insensitive
bool strEq(string a, string b);

// Finds the position of the close paren associated with the first open paren at or after start
int findCloseParen(string s, int start);

// Coloring function for optional use by image processor
// Takes a value from 0-255
// Offset it by the offset value, wrapping around.
// If <= min, return 0. if >= max return 0
// Otherwise make a sine interpolation, maxing at the midpoint
// If reverse, invert the number at the end
int scaleColor(int val, int minimum = 0, int maximum = 255, int offset = 0, bool reverse = false);

// Returns the substring from start index up to and including finish
string subByIndex(string s, int start, int finish);

void seedGenerator();
void seedGenerator(unsigned int n);

//Returns true with about chance probability
//chance should be between 0 and 1
bool random(double chance);

//Returns a number based on the chance distribution in probabilities
//If given [chanceA, chanceB], 2
//then chanceA probability to return 0, chanceB probability to return 1
//1-(chanceA+chanceB) probability to return 2
//num is the length of probabilities
int randomChoice(double* probabilities, int num);

//Returns a number between min and max
double randomDouble(double min, double max);

//Returns an integer [a, b]
int randomInteger(int a, int b);

//Returns an integer [0, a)
int randomInteger(int a);

int* randomPermutation(int n);

template<typename T>
inline T * arraySelect(T * array, int fromSize, int toSize)
{
	T* newArray = new T[toSize];
	if (fromSize == toSize) { //Transcribe all values
		for (int i = 0; i < fromSize; i++) {
			newArray[i] = array[i];
		}
	}
	else if (fromSize < toSize) { //Randomly spread elements across larger array, preserving order
		int* p = randomPermutation(toSize);
		bool* include = new bool[toSize];
		for (int i = 0; i < toSize; i++) {
			include[i] = false;
			newArray[i] = nullptr;
		}
		for (int i = 0; i < fromSize; i++) {
			include[p[i]] = true;
		}
		int j = 0;
		for (int i = 0; i < toSize; i++) {
			if(include[i])
				newArray[i] = array[j++];
		}
		delete[] p;
		delete[] include;
	}
	else { //Select a random subset of elements to include. Preserve order
		int* p = randomPermutation(fromSize);
		bool* include = new bool[fromSize];
		for (int i = 0; i < fromSize; i++) {
			include[i] = false;
		}
		for (int i = 0; i < toSize; i++) {
			include[p[i]] = true;
		}
		int j = 0;
		for (int i = 0; i < fromSize; i++) {
			if(include[i])
				newArray[j++] = array[i];
		}
		delete[] p;
		delete[] include;
	}
	return newArray;
}

//Free memory from Perlin Noise objects
void freePerlinNoise();
//Creates and stores PerlinNoise objects when given different random seeds
//Prevents wasted effort if the same seed is used multiple times
PerlinNoise* getPerlinNoise(unsigned int seed);