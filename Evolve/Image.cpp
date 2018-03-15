#include "Image.h"

Image::Image(int w, int h, Scalar * red) : w(w), h(h), red(red)
{
	c1 = new int[w*h];

	red->precomputeConstants();

	for (int x = 0; x < w; ++x) {
		red->precomputePartialX(x, w);
		for (int y = 0; y < h; ++y) {
			c1[w*y + x] = red->getGreyscaleValue(x, y, w);
		}
	}
}

Image::Image(int w, int h, Scalar * red, Scalar * green, Scalar * blue) : w(w), h(h), red(red), green(green), blue(blue)
{
	isGreyscale = false;
	c1 = new int[w*h];
	c2 = new int[w*h];
	c3 = new int[w*h];

	red->precomputeConstants();
	green->precomputeConstants();
	blue->precomputeConstants();

	for (int x = 0; x < w; ++x) {
		red->precomputePartialX(x, w);
		green->precomputePartialX(x, w);
		blue->precomputePartialX(x, w);
		for (int y = 0; y < h; ++y) {
			c1[w*y + x] = red->getGreyscaleValue(x, y, w);
			c2[w*y + x] = green->getGreyscaleValue(x, y, w);
			c3[w*y + x] = blue->getGreyscaleValue(x, y, w);
		}
	}
}

Image::~Image()
{
	delete[] c1;
	delete[] c2;
	delete[] c3;
}

void Image::drawImage(SDL_Renderer * renderer, int offsetX, int offsetY)
{
	int r, g, b;
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			r = c1[w*y + x];
			if (isGreyscale) {
				g = r;
				b = r;
			}
			else {
				g = c2[w*y + x];
				b = c3[w*y + x];
			}
			SDL_SetRenderDrawColor(renderer, r, g, b, 255);
			SDL_RenderDrawPoint(renderer, x + offsetX, y + offsetY);
		}
	}
}
