#include "Util.h"

int ratioToBin(double x)
{
	double d;
	return (int) (2056 * modf(x, &d));
	//return 0;
}

double binToRatio(int x)
{
	return (double)(x % 2056) / 2056.0;
}

double ratioAnd(double a, double b)
{
	return binToRatio(ratioToBin(a) & ratioToBin(b));
}

double ratioOr(double a, double b)
{
	return binToRatio(ratioToBin(a) | ratioToBin(b));
}

double ratioXor(double a, double b)
{
	return binToRatio(ratioToBin(a) ^ ratioToBin(b));
}

bool strEq(string a, string b)
{
	if (a.length() != b.length())
		return false;
	for (size_t i = 0; i < a.length(); i++) {
		if (toupper(a[i]) != toupper(b[i]))
			return false;
	}
	return true;
}

int findCloseParen(string s, int start)
{
	size_t n = start;
	int parenCount = 0;

	while (n < s.length()) {
		if (s[n] == '(') {
			parenCount = 1;
			n += 1;
			break;
		}
		n++;
	}

	while (n < s.length()) {
		if (s[n] == '(') {
			parenCount += 1;
		}
		if (s[n] == ')') {
			parenCount -= 1;
			if (!parenCount)
				return n;
		}
		n++;
	}

	return -1;
}

int scaleColor(int val, int minimum, int maximum, int offset, bool reverse)
{
	int c = (val + offset) % 255;
	int n = 0;
	if (c <= minimum)
		n = 0;
	else if (c >= maximum)
		n = 0;
	else {
		double r = ((double)(c)-(double)(minimum)) / ((double)(maximum)-(double)(minimum));
		r *= 3.141592 / 2.0;
		n = (int)(255 * sin(r));
	}
	if (reverse)
		n = 255 - n;
	return n;
}

string subByIndex(string s, int start, int finish)
{
	return s.substr(start, finish-start+1);
}

default_random_engine generator;
uniform_real_distribution<double> distribution(0.0, 1.0);

void seedGenerator()
{
	generator.seed(time(NULL));
}

void seedGenerator(unsigned int n)
{
	generator.seed(n);
}

bool random(double chance)
{
	if (distribution(generator) < chance)
		return true;
	return false;
}

int randomChoice(double * probabilities, int num)
{
	if (num <= 0)
		return 0;
	double roll = distribution(generator);
	double prob = 0;
	int i = 0;
	while (i < num) {
		prob += probabilities[i];
		if (roll < prob)
			return i;
		i++;
	}
	return num;
}

double randomDouble(double min, double max)
{
	uniform_real_distribution<double> d(min, max);
	return d(generator);
}

//can be improved to use <random>
int randomInteger(int a, int b)
{
	uniform_int_distribution<int> d(a, b);
	return d(generator);
}

int randomInteger(int a)
{
	return randomInteger(0,a-1);
}

int * randomPermutation(int n)
{
	vector<int> v(n);
	iota(v.begin(), v.end(), 0);
	shuffle(v.begin(), v.end(), generator);

	//Why am I insisiting on using arrays instead of vectors, despite even using them here?
	//No particular reason
	//I suppose I'm just trying to ensure my pointer skills are on point

	int* p = new int[n];
	for (int i = 0; i < n; i++) {
		p[i] = v[i];
	}
	return p;
}


//Naive way to ensure that not too many noise objects are stored at once
const int MAX_NOISE = 1000;

map<double, PerlinNoise*> noise;

void freePerlinNoise()
{
	for (map<double, PerlinNoise*>::iterator it = noise.begin(); it != noise.end(); ++it)
		delete it->second;
	noise.clear();
}

PerlinNoise * getPerlinNoise(unsigned int seed)
{
	if (noise.find(seed) == noise.end()) {
		// not found
		if (noise.size() >= MAX_NOISE) {
			freePerlinNoise(); //Naive way to ensure no overburdened memory
		}
		noise[seed] = new PerlinNoise(seed);
		
	}
	return noise[seed];
}