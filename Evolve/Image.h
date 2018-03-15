#pragma once
#include "Scalar.h"
#include <SDL.h>

//Image computes a pixel map from Scalar functions
//If one Scalar function is provided, it will be a greyscale image
//If three are provided, each one will be given a color channel
class Image
{
	int w, h;
	Scalar* red;
	Scalar* green;
	Scalar* blue;
	bool isGreyscale = true;
	int* c1;
	int* c2;
	int* c3;
public:
	Image(int w, int h, Scalar* red);
	Image(int w, int h, Scalar* red, Scalar* green, Scalar* blue);
	~Image();

	//Writes the pixels to the SDL_Renderer
	//If an offset is provided, apply the offset
	void drawImage(SDL_Renderer * renderer, int offsetX = 0, int offsetY = 0);
};

