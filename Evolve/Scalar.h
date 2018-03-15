//
// The symbolic expression grammar
// Unlike in Karl Sims' paper, all functions map scalar tupples to scalars. There are no vectors
//

#pragma once

#include <string>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include "Util.h"
using namespace std;

//Major efficiency boost, prevents noise seed from being a variable by setting all X and Y in the subtree of the noise seed value to 0
//Set false if more authentic math is desired
#define CONSTANT_SEED true

class Scalar
{
public:
	virtual ~Scalar() {}

	virtual double getValue(double x, double y) = 0;
	int getGreyscaleValue(int x, int y, int width) {
		double ratioX = 2*((double)x) / ((double)width) - 1.0;
		double ratioY = 2*((double)y) / ((double)width) - 1.0;
		int out = (int)(255 * getValue(ratioX, ratioY));
		//Clamp
		return min(255, max(0, out));
	}

	virtual int getHeight() = 0;
	virtual int getSize(bool includeLeaves = true) = 0;
	//Returns nullptr if mutating in place (only child nodes mutated)
	//Otherwise, return the replacement Scalar*
	virtual Scalar* mutate(int depth, int max_depth) = 0;
	virtual void _subMutate(int depth, int max_depth) { }	//Mutate all childern. Only FuncScalars have children
	virtual string repr() = 0;
	virtual Scalar* copy() = 0;	//Deep Copy
	virtual Scalar* shallowCopy(bool prepForDeletion = false) { return copy(); } //Only different for FuncScalars

	virtual bool _precomputeConstants(double &out) = 0; //Optimizations
	virtual bool _precomputePartialX(double x, double &out) = 0;

	void precomputeConstants() {
		reset();
		double d = 0;
		_precomputeConstants(d);
	}
	
	void precomputePartialX(int x, int width) {
		double d = 0;
		double ratioX = 2 * ((double)x) / ((double)width) - 1.0;
		_precomputePartialX(ratioX, d);
	}

	void reset() {};

	//Used as helper function to select a random node
	virtual Scalar* _getNthNode(int goal, int& count, bool includeLeaves = true) {
		if(includeLeaves)
			count++;
		if (goal == count )
			return this;
		else
			return nullptr;
	}
};

// Constant
class Constant : public Scalar
{
	double c;
public:
	Constant(double c) : c(c) {}
	~Constant() {}
	double getValue(double x, double y) {
		return c;
	}
	int getHeight() {
		return 1;
	}
	int getSize(bool includeLeaves = true) {
		if (includeLeaves)
			return 1;
		else
			return 0;
	}
	Scalar* mutate(int depth, int max_depth);
	string repr() {
		return to_string(c);
	}
	Scalar* copy() {
		return new Constant(c);
	}
	bool _precomputeConstants(double &out) {
		out = c;
		return true;
	}
	bool _precomputePartialX(double x, double &out) {
		out = c;
		return true;
	}
};

//Abstract Scalar subclasses
class VarScalar : public Scalar
{
public:
	~VarScalar() {}
	int getHeight() {
		return 1;
	}
	int getSize(bool includeLeaves = true) {
		if (includeLeaves)
			return 1;
		else
			return 0;
	}
	Scalar* mutate(int depth, int max_depth);
	bool _precomputeConstants(double &out) {
		return false;
	}
};

class FuncScalar : public Scalar
{
protected:
	int n;
	Scalar** args; //Array of Scalar pointers
	prestat* isPre; //Array of bools to tell if vals have been constant precomputed
	double* pre; //Constant precomputes
public:
	FuncScalar(Scalar** parameters, int numParameters) {
		n = numParameters;
		args = parameters;
		isPre = new prestat[n];
		pre = new double[n];
		for (int i = 0; i < n; i++) {
			isPre[i] = no;
			pre[i] = 0;
		}
	}
	//WARNING! WILL DELETE ALL CHILDREN AS WELL
	//Call extractParameters first is this is not the desired solution
	~FuncScalar() {
		for (int i = 0; i < n; i++) {
			delete args[i];
		}
		delete[] args;
		delete[] isPre;
		delete[] pre;
	}
	//WARNING! Only call immediately before deletion
	//Will return list of all parameters
	//If this is not called before deletion, all the parameters functions will be recursively deleted
	Scalar** extractParameters() {
		Scalar** extracted = new Scalar*[n];
		for (int i = 0; i < n; i++) {
			extracted[i] = args[i];
			args[i] = nullptr;
		}
		return extracted;
	}
	//WARNING! Only call immediately before deletion
	//Will return a single parameter and prevent it from being deep deleted
	//Parameters that are not extracted will be recusrively deleted when "this" is deleted
	Scalar* extractSingleParameter(int k) {
		Scalar* extracted = args[k];
		args[k] = nullptr;
		return extracted;
	}
	//Will delete paramaeter k and replace it with newParameter
	void replaceSingleParameter(Scalar* newParameter, int k) {
		delete args[k];
		args[k] = newParameter;
	}
	//Will delete a single parameter chosen at random and replace it with newParameter
	void replaceSingleParameter(Scalar* newParameter) {
		replaceSingleParameter(newParameter, randomInteger(n));
	}
	int getHeight() {
		int m = 0;
		for (int i = 0; i < n; i++) {
			int h = args[i]->getHeight();
			if (h > m)
				m = h;
		}
		return m + 1;
	}
	int getSize(bool includeLeaves = true) {
		int m = 1;
		for (int i = 0; i < n; i++) {
			m += args[i]->getSize(includeLeaves);
		}
		return m;
	}
	Scalar * mutate(int depth, int max_depth);

	//Mutates all the children
	void _subMutate(int depth, int max_depth);

	//If it is already precomputed, return that, otherwise get the parameter's value
	double getParamValue(int i, double x, double y) {
		if ((isPre[i] == no)) {
			return args[i]->getValue(x, y);
		}
		else {
			return pre[i];
		}
	}

	bool _precomputeConstants(double &out) {
		bool computed = true;
		for (int i = 0; i < n; i++) {
			if (args[i]->_precomputeConstants(pre[i])) {
				isPre[i] = con;
			}
			else {
				computed = false;
			}
		}
		if (computed) {
			out = getValue(0, 0);
		}
		return computed;
	}

	bool _precomputePartialX(double x, double &out) {
		for (int i = 0; i < n; i++) {
			if (isPre[i] == xVar) {
				isPre[i] = no;
				pre[i] = 0;
			}
		}
		bool computed = true;
		for (int i = 0; i < n; i++) {
			if (isPre[i] != con) {
				if (args[i]->_precomputePartialX(x, pre[i])) {
					isPre[i] = xVar;
				}
				else {
					computed = false;
				}
			}
		}
		if (computed) {
			out = getValue(x, 0);
		}
		return computed;
	}

	void reset() {
		for (int i = 0; i < n; i++) {
			args[i]->reset();
			isPre[i] = no;
			pre[i] = 0;
		}
	}

	//Used as helper function to select a random node
	Scalar* _getNthNode(int goal, int& count, bool includeLeaves = true) {
		count++;
		if (goal == count)
			return this;
		else {
			for (int i = 0; i < n; i++) {
				Scalar* r = args[i]->_getNthNode(goal, count, includeLeaves);
				if (goal == count)
					return r;
			}
		}
		return nullptr;
	}
};

template <typename D>
class _FuncScalar : public FuncScalar
{
public:
	_FuncScalar(Scalar** parameters, int numParameters) : FuncScalar(parameters, numParameters) {}
	//Deep Copy
	Scalar* copy() {
		Scalar** copiedParameters = new Scalar*[n];
		for (int i = 0; i < n; i++) {
			copiedParameters[i] = args[i]->copy();
		}
		return new D(copiedParameters);
	}
	//Shallow copy
	//If prepForDeletion is set to true, it will be safe to delete "this" after calling the function
	//		It will not be possible to use "this" after prepping for deletion
	Scalar* shallowCopy(bool prepForDeletion=false) {
		Scalar** copiedParameters;
		if (prepForDeletion) {
			copiedParameters = extractParameters();
		}
		else {
			copiedParameters = new Scalar*[n];
			for (int i = 0; i < n; i++) {
				copiedParameters[i] = args[i];
			}
		}
		return new D(copiedParameters);
	}
};


// Coordinate variables
class X : public VarScalar
{
public:
	double getValue(double x, double y) {
		return x;
	}
	string repr() {
		return "X";
	}
	Scalar* copy() {
		return new X();
	}
	bool _precomputePartialX(double x, double &out) {
		out = x;
		return true;
	}
};

class Y : public VarScalar
{
public:
	double getValue(double x, double y) {
		return y;
	}
	string repr() {
		return "Y";
	}
	Scalar* copy() {
		return new Y();
	}
	bool _precomputePartialX(double x, double &out) {
		return false;
	}
};



// Basic operations
class Add : public _FuncScalar<Add>
{
public:
	Add(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return getParamValue(0, x, y) + getParamValue(1, x, y);
	}
	string repr() {
		return "( + " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Subtract : public _FuncScalar<Subtract>
{
public:
	Subtract(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return getParamValue(0, x, y) - getParamValue(1, x, y);
	}
	string repr() {
		return "( - " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Multiply : public _FuncScalar<Multiply>
{
public:
	Multiply(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return getParamValue(0, x, y) * getParamValue(1, x, y);
	}
	string repr() {
		return "( * " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Divide : public _FuncScalar<Divide>
{
public:
	Divide(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		double q = getParamValue(0, x, y);
		double w = getParamValue(1, x, y);
		if (!w) {
			w += 0.00000001; //Stop division by zero errors
		}
		return q / w;
	}
	string repr() {
		return "( / " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Modulo : public _FuncScalar<Modulo>
{
public:
	Modulo(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return fmod(getParamValue(0, x, y), getParamValue(1, x, y));
	}
	string repr() {
		return "( % " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Minimum : public _FuncScalar<Minimum>
{
public:
	Minimum(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return min(getParamValue(0, x, y), getParamValue(1, x, y));
	}
	string repr() {
		return "( min " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Maximum : public _FuncScalar<Maximum>
{
public:
	Maximum(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return max(getParamValue(0, x, y), getParamValue(1, x, y));
	}
	string repr() {
		return "( max " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};


// Simple unary functions
class Floor : public _FuncScalar<Floor>
{
public:
	Floor(Scalar** a) : _FuncScalar(a, 1) {}
	double getValue(double x, double y) {
		return floor(getParamValue(0, x, y));
	}
	string repr() {
		return "( floor " + args[0]->repr() + " )";
	}
};

class Fraction : public _FuncScalar<Fraction>
{
public:
	Fraction(Scalar** a) : _FuncScalar(a, 1) {}
	double getValue(double x, double y) {
		double discard;
		return modf(getParamValue(0, x, y), &discard);
	}
	string repr() {
		return "( fraction " + args[0]->repr() + " )";
	}
};

class Absolute : public _FuncScalar<Absolute>
{
public:
	Absolute(Scalar** a) : _FuncScalar(a, 1) {}
	double getValue(double x, double y) {
		return fabs(getParamValue(0, x, y));
	}
	string repr() {
		return "( abs " + args[0]->repr() + " )";
	}
};


//Approximated Bitwise Operations
class And : public _FuncScalar<And>
{
public:
	And(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return ratioAnd(getParamValue(0, x, y), getParamValue(1, x, y));
	}
	string repr() {
		return "( & " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Or : public _FuncScalar<Or>
{
public:
	Or(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return ratioOr(getParamValue(0, x, y), getParamValue(1, x, y));
	}
	string repr() {
		return "( | " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

class Xor : public _FuncScalar<Xor>
{
public:
	Xor(Scalar** a) : _FuncScalar(a, 2) {}
	double getValue(double x, double y) {
		return ratioXor(getParamValue(0, x, y), getParamValue(1, x, y));
	}
	string repr() {
		return "( ^ " + args[0]->repr() + " " + args[1]->repr() + " )";
	}
};

//Trigonometric functions, normalized to 0-1 approximately based on rotations
class Sine : public _FuncScalar<Sine>
{
public:
	Sine(Scalar** a) : _FuncScalar(a, 1) {}
	double getValue(double x, double y) {
		return sin(getParamValue(0, x, y)*3.14)/2.0 + 0.5;
	}
	string repr() {
		return "( sin " + args[0]->repr() + " )";
	}
};

class Cosine : public _FuncScalar<Cosine>
{
public:
	Cosine(Scalar** a) : _FuncScalar(a, 1) {}
	double getValue(double x, double y) {
		return cos(getParamValue(0, x, y)*3.14) / 2.0 + 0.5;
	}
	string repr() {
		return "( cos " + args[0]->repr() + " )";
	}
};


// a>=b?c:d
class IfGT : public _FuncScalar<IfGT>
{
public:
	IfGT(Scalar** a) : _FuncScalar(a, 4) {}
	double getValue(double x, double y) {
		if (getParamValue(0, x, y) >= getParamValue(1, x, y)) {
			return getParamValue(2, x, y);
		}
		else
		{
			return getParamValue(3, x, y);
		}
	}
	string repr() {
		return "( if " + args[0]->repr() + " " + args[1]->repr() + " " + args[2]->repr() + " " + args[3]->repr() + " )";
	}
};

//Noise function, takes a seed value, and three coordinate values
//( noise seed x y z )
//Based on Perlin Noise
//if seed is not a constant, there are efficiency issues
//CONSTANT_SEED forces the seed to be a constant by assigning X and Y of the seed parameter to 0
class Noise : public _FuncScalar<Noise>
{
public:
	double lastVal = 0;
	Noise(Scalar** a) : _FuncScalar(a, 4) {}
	double getValue(double x, double y) {
		double seed;
		if (CONSTANT_SEED) {
			seed = getParamValue(0, 0, 0);
		}
		else {
			seed = getParamValue(0, x, y);
		}
		PerlinNoise* pn = getPerlinNoise((unsigned int)floor((2056 * seed)));
		return pn->noise(getParamValue(1, x, y), getParamValue(2, x, y), getParamValue(3, x, y));
	}
	string repr() {
		return "( noise " + args[0]->repr() + " " + args[1]->repr() + " " + args[2]->repr() + " " + args[3]->repr() + " )";
	}
};


//Generate random Scalars with random arguments

Scalar* randomScalar();

Constant* randomConstant();

VarScalar* randomVarScalar();

//Paramaters used for preseeding arguments
FuncScalar* randomFuncScalar(Scalar** seedParameters=nullptr, int num=0);


//Returns a random node in the tree
//If includes leaves is false, will only return nonleaf nodes
Scalar* getRandomNode(Scalar* tree, bool includeLeaves = true);

//Produces offspring from two Scalars
Scalar* breed(Scalar* a, Scalar* b);

//Produces n offspring
Scalar** breed(Scalar* a, Scalar* b, int n, bool mutate=false);