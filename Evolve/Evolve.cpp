﻿// Evolve.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "Util.h"
#include "Scalar.h"
#include <SDL.h>
#include <string>
#include <sstream>
#include <regex>
#include "Image.h"
using namespace std;

#define MAX_FUNC 4
#define WINDOW_WIDTH 400

Scalar * parseScalar(string s)
{
	if (tolower(s[0]) == 'x')
		return new X();
	else if (tolower(s[0]) == 'y')
		return new Y();
	else if (s[0] == '(') {
		int finish = s.find(" ", 2) - 1;
		string func = subByIndex(s, 2, finish);

		string token[MAX_FUNC];

		for (int i = 0; i < MAX_FUNC; i++) {
			token[i] = "";
		}

		int start = finish + 2;
		int n = 0;

		while (s[start] != ')') {
			if (s[start] == '(') {
				finish = findCloseParen(s, start);
			}
			else {
				finish = s.find(" ", start) - 1;
			}

			token[n++] = subByIndex(s, start, finish);

			start = finish + 2;
		}

		Scalar** parameters = new Scalar*[n];
		for (int i = 0; i < n; i++) {
			parameters[i] = parseScalar(token[i]);
		}

		if (strEq(func, "+"))
			return new Add(parameters);

		if (strEq(func, "-"))
			return new Subtract(parameters);

		if (strEq(func, "*"))
			return new Multiply(parameters);

		if (strEq(func, "/"))
			return new Divide(parameters);

		if (strEq(func, "%"))
			return new Modulo(parameters);

		if (strEq(func, "min"))
			return new Minimum(parameters);

		if (strEq(func, "max"))
			return new Maximum(parameters);

		if (strEq(func, "floor"))
			return new Floor(parameters);

		if (strEq(func, "fraction"))
			return new Fraction(parameters);

		if (strEq(func, "abs"))
			return new Absolute(parameters);

		if (strEq(func, "&"))
			return new And(parameters);

		if (strEq(func, "|"))
			return new Or(parameters);

		if (strEq(func, "^"))
			return new Xor(parameters);

		if (strEq(func, "sin"))
			return new Sine(parameters);

		if (strEq(func, "cos"))
			return new Cosine(parameters);

		if (strEq(func, "if"))
			return new IfGT(parameters);

		if (strEq(func, "noise"))
			return new Noise(parameters);

		return nullptr;
	}
	//Was a constant
	double d;
	try {
		d = stod(s);
	}
	catch (const std::invalid_argument& ia) {
		cerr << "Something whent wrong in the parser.\n";
		cerr << s << "\n";
	}
	return new Constant(d);
}

Scalar* parse(string s) {
	s = regex_replace(s, regex("^ +| +$|( ) +"), "$1"); //Remove leading, trailing, and extra whitespace
	return parseScalar(s);
}

Scalar* mutate(Scalar* in) {
	return in->mutate(0, in->getHeight());
}

//Creates a random image of at least depth n
Scalar* randomFunction(int n=0) {
	if (n == 0) {
		return randomScalar();
	}
	
	Scalar* image = randomFuncScalar();

	while (image->getHeight() < n) {
		delete image;
		image = randomScalar();
	}

	return image;
}

void clear() {
	for (int i = 0; i<100; i++)
		cout << "\n";
}

void mutationDemo() {
	clear();
	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;

	//These lines were used to test preset images

	//Scalar* x = new X();
	//Scalar* y = new Y();
	//string s = "( cos ( ^ ( abs ( / X 2 ) ) ( sin ( * ( + ( / Y 2 ) -0.500000 ) 2.000000 ) ) ) )";
	//string s = "( cos ( / ( abs ( / X 2 ) ) ( sin ( * ( + ( / Y 2 ) -0.500000 ) 2.000000 ) ) ) )";
	//string s = "( - 1 ( cos ( ^ ( abs ( / X 2 ) ) ( sin ( * ( + ( / Y 2 ) -0.500000 ) 2.000000 ) ) ) ) )";
	//string t = "( * ( & x y ) 50 )";

	//string crazy = "( cos ( / ( cos ( / X ( & 0.776632 ( - ( & ( | X ( / X 4.531517 ) ) ( max Y ( abs Y ) ) ) X ) ) ) ) ( - ( * ( + ( / X 2.000000 ) -0.500000 ) 2.000000 ) Y ) ) )";

	//string amazing = "( fraction ( | ( + ( noise 0.414332 ( min ( cos ( - Y Y ) ) ( abs Y ) ) 7.076917 ( if 8.053144 ( % ( sin ( & 2.628216 ( floor -8.215797 ) ) ) ( ^ ( if ( | 0.802172 Y ) ( | ( min ( * Y ( % ( abs ( % 4.220468 ( cos ( ^ Y X ) ) ) ) ( ^ Y ( cos X ) ) ) ) ( sin ( / ( min 9.875528 Y ) 5.439929 ) ) ) ( abs Y ) ) ( / X Y ) ( | ( cos X ) -3.212698 ) ) ( floor 5.960819 ) ) ) X ( max 7.680117 X ) ) ) 4.685479 ) ( | 0.298756 ( abs ( max X ( ^ Y ( ^ -1.690544 ( ^ X ( - X X ) ) ) ) ) ) ) ) )";

	//string noise = "( noise ( + X Y ) ( * X 4 ) ( * Y 4 ) 0 )";

	//s = "( if " + s + " .8 ( & x y ) " + s + " )"; // Cut to reveal board
	//s = "( if .5 " + t + " 0 " + s + " )"; // Serpenski

	//Scalar* func = parse(amazing);

	Scalar* func = randomFunction(5);

	func->precomputeConstants();

	cout << func->repr() << "\n\n";

	Image* image = new Image(WINDOW_WIDTH, WINDOW_WIDTH, func);

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	
	image->drawImage(renderer);

	SDL_RenderPresent(renderer);
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			//if (event.type == SDL_QUIT)
			if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
			{
				done = true;
				break;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {

				Scalar* newFunc = nullptr;

				if (event.button.button == SDL_BUTTON_LEFT)
					newFunc = mutate(func);
				else if (event.button.button == SDL_BUTTON_RIGHT)
					newFunc = randomFunction(5);

				if (newFunc) {
					delete func;
					func = newFunc;
				}

				delete image;
				image = new Image(WINDOW_WIDTH, WINDOW_WIDTH, func);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);
				
				image->drawImage(renderer);

				SDL_RenderPresent(renderer);

				//Clear event queue to prevent too many clicks
				//Only care about quit events
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
					{
						done = true;
						break;
					}
				}

				cout << func->repr() << "\n\n";
			}
		}
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void printColorFunction(Scalar* r, Scalar* g, Scalar* b) {
	cout << "R: " << r->repr() << "\n";
	cout << "G: " << g->repr() << "\n";
	cout << "B: " << b->repr() << "\n\n";
}

void colorDemo() {
	clear();
	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;

	Scalar* red = randomFunction(2);
	Scalar* green = randomFunction(2);
	Scalar* blue = randomFunction(2);

	Image* image = new Image(WINDOW_WIDTH, WINDOW_WIDTH, red, green, blue);

	printColorFunction(red, green, blue);

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	
	image->drawImage(renderer);

	SDL_RenderPresent(renderer);
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
			{
				done = true;
				break;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {

				Scalar* newRed = nullptr;
				Scalar* newGreen = nullptr;
				Scalar* newBlue = nullptr;

				if (event.button.button == SDL_BUTTON_LEFT) {
					newRed = mutate(red);
					newGreen = mutate(green);
					newBlue = mutate(blue);
				}
				else if (event.button.button == SDL_BUTTON_RIGHT) {
					newRed = randomFunction(2);
					newGreen = randomFunction(2);
					newBlue = randomFunction(2);
				}

				if (newRed) {
					delete red;
					red = newRed;
				}
				if (newGreen) {
					delete green;
					green = newGreen;
				}
				if (newBlue) {
					delete blue;
					blue = newBlue;
				}

				delete image;
				image = new Image(WINDOW_WIDTH, WINDOW_WIDTH, red, green, blue);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);

				image->drawImage(renderer);

				SDL_RenderPresent(renderer);

				//Clear event queue to prevent too many clicks
				//Only care about quit events
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
					{
						done = true;
						break;
					}
				}

				printColorFunction(red, green, blue);
			}
		}
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void evolutionDemo() {
	clear();
	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;

	SDL_Rect* rect = new SDL_Rect();

	Scalar* population[16];

	for (int i = 0; i < 16; i++) {
		population[i] = randomFunction(2);
	}

	Image* images[16];
	bool selected[16];
	for (int i = 0; i < 16; i++) {
		images[i] = new Image(100, 100, population[i]);
		selected[i] = false;
	}

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(440, 440, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	for(int i=0; i<4;i++)
		for(int j=0;j<4;j++)
			images[i*4+j]->drawImage(renderer,110*j+5,110*i+5);

	SDL_RenderPresent(renderer);

	bool done = false;

	int numSelected = 0;

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
			{
				done = true;
				break;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				int j = event.button.x / 110;
				int i = event.button.y / 110;

				if (selected[i * 4 + j]) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					selected[i * 4 + j] = false;
					numSelected -= 1;
				}
				else {
					SDL_SetRenderDrawColor(renderer, 250, 250, 210, 255);
					selected[i * 4 + j] = true;
					numSelected += 1;
					cout << population[i * 4 + j]->repr() << "\n\n";
				}

				for (int q = 0; q < 5; q++) {
					rect->x = 110 * j + q;
					rect->y = 110 * i + q;
					rect->w = 110 - (2*q);
					rect->h = 110 - (2*q);
					SDL_RenderDrawRect(renderer, rect);
				}

				SDL_RenderPresent(renderer);
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					done = true;
					break;
				}
				else {
					if (numSelected != 2) {
						cout << "Please select exactly two images.\n\n";
					}
					else {
						Scalar* mother;
						Scalar* father;

						bool z = true;

						for (int i = 0; i < 16; i++) {
							if (selected[i]) {
								if (z) {
									mother = population[i]->copy();
									z = false;
								}
								else {
									father = population[i]->copy();
								}
							}
							delete population[i];
							delete images[i];
						}

						//Elitism
						population[0] = mother;
						population[1] = father;

						//Children
						Scalar** children = breed(mother, father, 12, true);
						for (int i = 0; i < 12; i++) {
							population[i + 2] = children[i];
						}
						delete[] children;

						//New random elements
						population[14] = randomFunction(2);
						population[15] = randomFunction(2);


						//Generate images from functions
						for (int i = 0; i < 16; i++) {
							images[i] = new Image(100, 100, population[i]);
							selected[i] = false;
						}

						numSelected = 0;

						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
						SDL_RenderClear(renderer);

						for (int i = 0; i<4; i++)
							for (int j = 0; j<4; j++)
								images[i * 4 + j]->drawImage(renderer, 110 * j + 5, 110 * i + 5);

						SDL_RenderPresent(renderer);

						cout << "\n\n\n";

						//Clear event queue to prevent too many clicks
						//Only care about quit events
						while (SDL_PollEvent(&event)) {
							if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
							{
								done = true;
								break;
							}
						}
					}
				}
			}
		}
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void help() {
	clear();
	cout << "This program serves as an implementation of one of the algorithms described in Karl Sims' 1991 Paper:\n"
		<< "\"Artificial Evolution for Computer Graphics.\"\n\n"
		<< "Three demos are provided: a Mutation Demo, a Color Demo, and a simple Evolution Demo.\n\n"
		<< "Type anything to continue.\n";

	string s;
	cin >> s;

	clear();
	cout << "The Mutation Demo serves to demonstrate symbolic expressions and their mutations.\n\n"
		<< "Launching the demo will produce a random expression, denoted in prefix notation,\n"
		<< "as well as the associated grayscale image.\n\n"
		<< "Left clicking on the image will mutate the expression randomly and regenerate the image.\n\n"
		<< "Right clicking on the image will create a new, random expression from scratch.\n\n"
		<< "Pressing any key or closing the window will end the demo.\n\n"
		<< "Please note that there are a large number of graphically trivial expressions, \n"
		<< "so you may be required to click a few times to get anything interesting to appear.\n\n"
		<< "Please also note that some expressions are computationally complex and may require a few seconds to render.\n\n"
		<< "Type anything to continue.\n";

	cin >> s;

	clear();
	cout << "The Color Demo is a simple variation of the Mutation Demo.\n\n"
		<< "Rather than a single symbolic expression, the Color demo produces three: one for each color channel.\n\n"
		<< "The demo works the same, otherwise. Left clicking will mutate the expressions,\n"
		<< "right clicking will regenerate them from scratch,\n"
		<< "and pressing any key or exiting the window will end the demo.\n\n"
		<< "Colored images are more visually complex, so the effect of a single mutation is less clear.\n\n"
		<< "Colored images are also three times slower to render.\n\n"
		<< "Type anything to continue.\n";

	cin >> s;

	clear();
	cout << "The Evolution Demo serves to demonstrate the genetic algorithm.\n\n"
		<< "When the demo begins, you will be presented with sixteen randomly generated images.\n\n"
		<< "You may select an image by clicking on it. Clicking it a second time will deselect it.\n\n"
		<< "When you select an image, the expression that generated that image will be printed to the console.\n\n"
		<< "After you have selected exactly two images, you may breed them by pressing any key.\n\n"
		<< "Breeding the images will produce sixteen new images:\n"
		<< "the two chosen parents (providing elitism to the algorithm),\n"
		<< "twelve children produced by crossover and mutuation,\n"
		<< "and two completely random entries (to prevent stagnation from the small population size).\n\n"
		<< "You may repeat this process as often as you desire.\n\n"
		<< "Close the window or press the escape key to exit the demo.\n\n"
		<< "Type anything to go back to the main menu.\n\n";
	cin >> s;

	clear();
}

int main(int argc, char* args[]) {
	seedGenerator();
	//seedGenerator(1);

	int i = 0;

	cout << "Evolve: A C++ implementation of Karl Sims' \"Artificial Evolution for Computer Graphics\"\nby Bruce Bland\n\n";

	while (i != -1) {
		clear();
		cout << "Main Menu:\nHelp: 1\nMutation Demo: 2\nColor Demo: 3\nEvolution Demo: 4\nExit: -1\n";
		string input = "";
		getline(cin, input);
		stringstream stream(input);
		if (!(stream >> i))
			continue;
		if (i == 2)
			mutationDemo();
		if (i == 3)
			colorDemo();
		if (i == 4)
			evolutionDemo();
		if (i == 1)
			help();
	}

	return EXIT_SUCCESS;
}
