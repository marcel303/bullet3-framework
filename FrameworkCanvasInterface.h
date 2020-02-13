#pragma once

#include "CommonInterfaces/Common2dCanvasInterface.h"
#include "framework.h"
#include <map>
#include <stdint.h>

struct FrameworkCanvas
{
	uint8_t * pixels = nullptr;
	int sx = 0;
	int sy = 0;
	
	GxTextureId textureId = 0;
	
	~FrameworkCanvas()
	{
		freeTexture(textureId);
		
		delete [] pixels;
		pixels = nullptr;
	}
	
	uint8_t * getLine(const int y) { return pixels + y * sx * 4; }
};

class FrameworkCanvasInterface : public Common2dCanvasInterface
{
public:
	std::map<int, FrameworkCanvas*> m_canvases;
	int m_nextCanvasId = 1;
	
public:
	virtual ~FrameworkCanvasInterface() override final;
	
	virtual int createCanvas(const char* canvasName, int width, int height, int xPos, int yPos) override final;
	virtual void destroyCanvas(int canvasId) override final;
	virtual void setPixel(int canvasId, int x, int y, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) override final;
	virtual void getPixel(int canvasId, int x, int y, unsigned char& red, unsigned char& green, unsigned char& blue, unsigned char& alpha) override final;

	virtual void refreshImageData(int canvasId) override final;
	
	void drawCanvas(const int x, const int y) const;
};
