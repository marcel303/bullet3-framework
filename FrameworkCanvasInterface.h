#pragma once

#include "CommonInterfaces/Common2dCanvasInterface.h"

class FrameworkCanvasInterface : public Common2dCanvasInterface
{
public:
	virtual ~FrameworkCanvasInterface() override final;
	virtual int createCanvas(const char* canvasName, int width, int height, int xPos, int yPos) override final;
	virtual void destroyCanvas(int canvasId) override final;
	virtual void setPixel(int canvasId, int x, int y, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) override final;
	virtual void getPixel(int canvasId, int x, int y, unsigned char& red, unsigned char& green, unsigned char& blue, unsigned char& alpha) override final;

	virtual void refreshImageData(int canvasId) override final;
};
