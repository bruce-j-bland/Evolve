#include "Scalar.h"
#include <iostream>
using namespace std;

//Odds that an individual node will mutate
#define CHANCE_FUNC ((1.0 - 1.0 / (depth + 2.0)) / (max_depth)) * 0.75

//Odds that a mutation will be completely new
#define COMPLETE_MUTATE 0.1

//Odds that a FuncScalar will mutate into a different function with the same parameters
#define FUNCTION_MUTATE 0.3

//Odds that a FuncScalar will become the argument of a new random function
#define PUSH_MUTATE 0.15

//Odds that a FuncScalar argument will be pulled out and replace the function
#define PULL_MUTATE 0.25

//Odds that an argument will be duplicated internally
#define CLONE_MUTATE 0.2

//Chance that a newly formed Scalar will be a constant. 0-1, higher means shorter trees
#define CONST_CHANCE 0.3

//Chance that a newly formed Scalar will be X or Y. 0-1, higher means shorter trees
#define VAR_CHANCE 0.3

Scalar * Constant::mutate(int depth, int max_depth)
{
	if (random(CHANCE_FUNC)) {
		if (random(COMPLETE_MUTATE)) {
			return randomScalar();
		}
		else {
			double m = ((rand() % 100) - 50.0) / 50.0;
			return new Constant(c + m);
		}
	}
	return nullptr;
}

Scalar* VarScalar::mutate(int depth, int max_depth)
{
	if (random(CHANCE_FUNC)) {
		if (random(COMPLETE_MUTATE)) {
			return randomScalar();
		}
		else {
			return randomVarScalar();
		}
	}
	return nullptr;
}


Scalar * FuncScalar::mutate(int depth, int max_depth)
{
	if (random(CHANCE_FUNC)) {
		double prob[] = { COMPLETE_MUTATE, FUNCTION_MUTATE, PUSH_MUTATE, PULL_MUTATE, CLONE_MUTATE };
		switch (randomChoice(prob, 5)) {
		case 0: { //Complete mutation
			return randomScalar();
		}

		case 1: { //Mutate just the function
			Scalar * * extractedParameters = extractParameters();
			FuncScalar* f = randomFuncScalar(extractedParameters, n);
			delete[] extractedParameters;
			f->_subMutate(depth, max(max_depth, depth+f->getHeight())); //Height might have changed from adding new variables
			return f;
		}
		
		case 2: { //Push the function into a new function
			Scalar ** pushParameters = new Scalar*[1];
			pushParameters[0] = shallowCopy(true);
			pushParameters[0]->_subMutate(depth + 1, max_depth + 1); //Assume you are in the largest child branch, does not affect probability much
			FuncScalar* f = randomFuncScalar(pushParameters, 1);
			delete[] pushParameters;
			return f;
		}
		case 3: { //Pull a function out of the parameters
			Scalar* pullParameter = extractSingleParameter(randomInteger(n));
			pullParameter->_subMutate(depth - 1, max_depth - 1); //Assume you are in the largest child branch, does not affect probability much
			return pullParameter;
		}
		case 4: { //Clone a parameter
			if (n <= 1) { //Impossible to clone
				_subMutate(depth, max_depth);
			}
			else {
				int* p = randomPermutation(n); //The first index will be copied into the second. The remainder will be unaffected

				//Recursively mutate all the parameters EXCEPT the one being replaced
				for (int i = 0; i < n; i++) {
					if (i != p[1]) {
						Scalar* newA = args[i]->mutate(depth + 1, max_depth);
						if (newA) {
							delete args[i];
							args[i] = newA;
						}
					}
					else { //It will be replaced, so it should be deleted
						delete args[i];
					}
				}

				//Replace with a copy
				args[p[1]] = args[p[0]]->copy();

				delete p;
				return nullptr;
			}
		}
		default: { //No mutations
			_subMutate(depth, max_depth);
		}
		}
	}
	else {
		_subMutate(depth, max_depth);
	}
	return nullptr;
}

void FuncScalar::_subMutate(int depth, int max_depth)
{
	for (int i = 0; i < n; i++) {
		Scalar* newA = args[i]->mutate(depth + 1, max_depth);
		if (newA) {
			delete args[i];
			args[i] = newA;
		}
	}
}

Scalar * randomScalar()
{
	double prob[] = { CONST_CHANCE, VAR_CHANCE };
	switch (randomChoice(prob, 2)) {
	case 0:
		return randomConstant();
	case 1:
		return randomVarScalar();
	case 2:
	default:
		return randomFuncScalar();
	}
}

Constant * randomConstant()
{
	return new Constant(randomDouble(-10,10));
}

VarScalar * randomVarScalar()
{
	if (random(0.5)) {
		return new X();
	}
	else {
		return new Y();
	}
}

/*
Unary Functions: (0-4)
	0: Floor
	1: Fraction
	2: Absolute
	3: Sine
	4: Cosine
Binary Functions: (5-14)
	5: Add
	6: Subtract
	7: Multiply
	8: Divide
	9: Modulo
	10: Minimum
	11: Maximum
	12: And
	13: Or
	14: Xor
Quaternary Functions: (15-16)
	15: IfGT
	16: Noise
*/
FuncScalar * randomFuncScalar(Scalar** seedParameters, int num)
{
	int choice = randomInteger(17);
	Scalar** parameters = nullptr;
	int newNum = 0;
	if (choice <= 4) {
		newNum = 1;
	}
	else if (choice <= 14) {
		newNum = 2;
	}
	else {
		newNum = 4;
	}

	if (num > 0) { //Fill in the unprovided parameters
		parameters = arraySelect(seedParameters, num, newNum);
		for (int i = 0; i < newNum; i++) {
			if (!parameters[i]) {
				parameters[i] = randomScalar();
			}
		}
	}
	else {
		parameters = new Scalar*[newNum];
		for (int i = 0; i < newNum; i++)
			parameters[i] = randomScalar();
	}

	switch (choice) {
	case 0:
		return new Floor(parameters);
	case 1:
		return new Fraction(parameters);
	case 2:
		return new Absolute(parameters);
	case 3:
		return new Sine(parameters);
	case 4:
		return new Cosine(parameters);
	case 5:
		return new Add(parameters);
	case 6:
		return new Subtract(parameters);
	case 7:
		return new Multiply(parameters);
	case 8:
		return new Divide(parameters);
	case 9:
		return new Modulo(parameters);
	case 10:
		return new Minimum(parameters);
	case 11:
		return new Maximum(parameters);
	case 12:
		return new And(parameters);
	case 13:
		return new Or(parameters);
	case 14:
		return new Xor(parameters);
	case 15:
		return new IfGT(parameters);
	case 16:
		return new Noise(parameters);
	}
	cerr << "Something has gone wrong creating a random FuncScalar.\n";
	return nullptr;
}

Scalar * getRandomNode(Scalar * tree, bool includeLeaves)
{
	int g = randomInteger(tree->getSize(includeLeaves)); //Index of the node in dfs ennumeration
	int count = -1;
	return tree->_getNthNode(g, count, includeLeaves);
}

//Helper function
//Grafts a portion from b to a
//a must have a height of at least 2
Scalar* _breed(Scalar * a, Scalar * b) {
	Scalar* child = a->copy();

	Scalar* graft = getRandomNode(b)->copy();

	FuncScalar* graftLocation = (FuncScalar*)getRandomNode(child, false);	//Not a leaf, so guarunteed to be a func scalar

	graftLocation->replaceSingleParameter(graft);

	return child;
}

//Creates a child by randomly graft a portion of a onto b, or a portion of b onto a
Scalar * breed(Scalar * a, Scalar * b)
{
	if (a->getHeight() <= 1) {
		if (b->getHeight() <= 1) { //No crossbreeding possible
			return a;
		}
		else {
			return _breed(b, a);
		}
	}
	else if (b->getHeight() <= 1) {
		return _breed(a, b);
	}
	else {
		if (random(0.5)) {
			return _breed(a, b);
		}
		else {
			return _breed(b, a);
		}
	}
	return nullptr;
}

Scalar ** breed(Scalar * a, Scalar * b, int n, bool mutate)
{
	Scalar** children = new Scalar*[n];
	for (int i = 0; i < n; i++) {
		children[i] = breed(a, b);
		if (mutate) {
			Scalar* mutation = children[i]->mutate(0, children[i]->getHeight());
			if (mutation) {
				delete children[i];
				children[i] = mutation;
			}
		}
	}
	return children;
}
