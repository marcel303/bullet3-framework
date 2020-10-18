#pragma once

#include "CommonInterfaces/Common2dCanvasInterface.h"
#include "framework.h"
#include "gx_texture.h"
#include <map>
#include <stdint.h>

struct Framework2dCanvas
{
	uint8_t * pixels = nullptr;
	int sx = 0;
	int sy = 0;
	
	GxTexture texture;
	
	~Framework2dCanvas()
	{
		texture.free();
		
		delete [] pixels;
		pixels = nullptr;
	}
	
	uint8_t * getLine(const int y) { return pixels + y * sx * 4; }
};

class Framework2dCanvasInterface : public Common2dCanvasInterface
{
public:
	std::map<int, Framework2dCanvas*> m_canvases;
	int m_nextCanvasId = 1;
	
public:
	virtual ~Framework2dCanvasInterface() override final;
	
	virtual int createCanvas(const char* canvasName, int width, int height, int xPos, int yPos) override final;
	virtual void destroyCanvas(int canvasId) override final;
	virtual void setPixel(int canvasId, int x, int y, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) override final;
	virtual void getPixel(int canvasId, int x, int y, unsigned char& red, unsigned char& green, unsigned char& blue, unsigned char& alpha) override final;

	virtual void refreshImageData(int canvasId) override final;
};
