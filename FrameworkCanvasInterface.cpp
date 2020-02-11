#include "FrameworkCanvasInterface.h"

FrameworkCanvasInterface::~FrameworkCanvasInterface()
{
}

int FrameworkCanvasInterface::createCanvas(const char* canvasName, int width, int height, int xPos, int yPos)
{
	return 0;
}

void FrameworkCanvasInterface::destroyCanvas(int canvasId)
{
}

void FrameworkCanvasInterface::setPixel(int canvasId, int x, int y, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
}

void FrameworkCanvasInterface::getPixel(int canvasId, int x, int y, unsigned char& red, unsigned char& green, unsigned char& blue, unsigned char& alpha)
{
	red = 1.f;
	green = 0.f;
	blue = 1.f;
	alpha = 1.f;
}

void FrameworkCanvasInterface::refreshImageData(int canvasId)
{
}
